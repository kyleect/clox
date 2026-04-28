#!/usr/bin/env bash

set -euo pipefail

BIN="${BIN:-./build/clox-test}"
DIFF="diff -u"

export CLOX_TEST="CLOX_TEST=true"

UPDATE=0
VERBOSE=0
FILTER=""

for arg in "$@"; do
    case "$arg" in
        --update)
            UPDATE=1
            ;;
        --verbose)
            VERBOSE=1
            ;;
        -v)
            VERBOSE=1
            ;;
        *)
            FILTER="$arg"
            ;;
    esac
done

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

SUITES=0
PASS=0
FAIL=0
SKIP=0
TOTAL=0

should_run() {
    local name="$1"

    if [[ -z "$FILTER" ]]; then
        return 0
    fi

    [[ "$name" == *"$FILTER"* ]]
}

run_test() {
    local label="$1"
    local expected="$2"
    local actual="$3"

    TOTAL=$((TOTAL + 1))

    if [[ $UPDATE -eq 1 ]]; then
        cp "$actual" "$expected"
        echo "  🔄 UPDATED: $label"
        PASS=$((PASS + 1))
        return
    fi

    if [[ ! -f "$expected" ]]; then
        echo "  🟨 SKIP $label (missing $(basename "$expected"))"
        SKIP=$((SKIP + 1))
        return
    fi

    if ! $DIFF "$expected" "$actual"; then
        echo "  ❌ $label"
        FAIL=$((FAIL + 1))
    else
        [[ $VERBOSE -eq 1 ]] && echo "  ✅ $label"
        PASS=$((PASS + 1))
    fi
}

run_exit_test() {
    local label="$1"
    local expected="$2"
    local actual_code="$3"

    TOTAL=$((TOTAL + 1))

    if [[ $UPDATE -eq 1 ]]; then
        echo "$actual_code" > "$expected"
        echo "  🔄 UPDATED: $label"
        PASS=$((PASS + 1))
        return
    fi

    if [[ ! -f "$expected" ]]; then
        echo "  🟨 SKIP $label (missing $(basename "$expected"))"
        SKIP=$((SKIP + 1))
        return
    fi

    expected_code=$(cat "$expected")

    if [[ "$actual_code" != "$expected_code" ]]; then
        message=""

        if  [[ "$actual_code" == "139" ]]; then
            message="SEGFAULT"
        elif [[ "$actual_code" == "134" ]]; then
            message="OUT OF BOUNDS"
        else
            message="got $actual_code, expected $expected_code"
        fi
        
        echo "  ❌ $label ($message)"
        FAIL=$((FAIL + 1))
    else
        [[ $VERBOSE -eq 1 ]] && echo "  ✅ $label"
        PASS=$((PASS + 1))
    fi
}

for file_in in ./tests/*.lox; do
    base=$(basename "$file_in" .lox)

    # Apply filter
    if ! should_run "$base"; then
        continue
    fi

    SUITES=$((SUITES + 1))

    expected_out="./tests/$base.lox.out"
    expected_err="./tests/$base.lox.err"
    expected_exit="./tests/$base.lox.exit"
    argv_file="./tests/$base.lox.argv"

    actual_out="$TMP_DIR/$base.lox.out"
    actual_err="$TMP_DIR/$base.lox.err"

    # Gather extra arguments if an .lox.argv file exists
    extra_args=()
    if [[ -f "$argv_file" ]]; then
        read -r -a extra_args < "$argv_file"
    fi

    set +e
    "$BIN" -f "$file_in" -- ${extra_args[@]:-} >"$actual_out" 2>"$actual_err"
    exit_code=$?
    set -e

    echo "🔬 $BIN $file_in ${extra_args[*]:-}"
    run_test      "stdout" "$expected_out" "$actual_out"
    run_test      "stderr" "$expected_err" "$actual_err"
    run_exit_test "exit code" "$expected_exit" "$exit_code"
    [[ $VERBOSE -eq 1 ]] && echo ""
done

echo
echo "Total: $TOTAL | Passed: $PASS | Failed: $FAIL | Skipped: $SKIP | Suites: $SUITES"

if [[ $FAIL -ne 0 ]]; then
    exit 1
fi