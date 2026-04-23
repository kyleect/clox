#!/usr/bin/env bash

set -euo pipefail

BIN="${BIN:-./build/clox-test}"
DIFF="diff -u"

UPDATE=0
FILTER=""

# Parse args (supports --update and optional filter)
for arg in "$@"; do
    case "$arg" in
        --update)
            UPDATE=1
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
        echo "🔄 UPDATED: $label"
        PASS=$((PASS + 1))
        return
    fi

    if [[ ! -f "$expected" ]]; then
        echo "🟨 SKIP $label (missing $(basename "$expected"))"
        SKIP=$((SKIP + 1))
        return
    fi

    if ! $DIFF "$expected" "$actual"; then
        echo "❌ FAIL $label"
        FAIL=$((FAIL + 1))
    else
        echo "✅ PASS $label"
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
        echo "🔄 UPDATED: $label"
        PASS=$((PASS + 1))
        return
    fi

    if [[ ! -f "$expected" ]]; then
        echo "🟨 SKIP $label (missing $(basename "$expected"))"
        SKIP=$((SKIP + 1))
        return
    fi

    expected_code=$(cat "$expected")

    if [[ "$actual_code" != "$expected_code" ]]; then
        echo "❌ FAIL $label (got $actual_code, expected $expected_code)"
        FAIL=$((FAIL + 1))
    else
        echo "✅ PASS $label"
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

    actual_out="$TMP_DIR/$base.lox.out"
    actual_err="$TMP_DIR/$base.lox.err"

    set +e
    "$BIN" "$file_in" >"$actual_out" 2>"$actual_err"
    exit_code=$?
    set -e

    run_test      "[out]  $base" "$expected_out" "$actual_out"
    run_test      "[err]  $base" "$expected_err" "$actual_err"
    run_exit_test "[exit] $base" "$expected_exit" "$exit_code"
done

echo
echo "Total: $TOTAL | Passed: $PASS | Failed: $FAIL | Skipped: $SKIP | Suites: $SUITES"

if [[ $FAIL -ne 0 ]]; then
    exit 1
fi