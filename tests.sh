#!/usr/bin/env bash

set -euo pipefail

BIN="${BIN:-./build/clox}"
DIFF="diff -u"

UPDATE=0
if [[ "${1:-}" == "--update" ]]; then
    UPDATE=1
fi

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

PASS=0
FAIL=0
SKIP=0
TOTAL=0

run_test() {
    local label="$1"
    local expected="$2"
    local actual="$3"

    TOTAL=$((TOTAL + 1))

    # Update mode
    if [[ $UPDATE -eq 1 ]]; then
        cp "$actual" "$expected"
        echo "🔄 UPDATED: $label"
        PASS=$((PASS + 1))
        return
    fi

    # Missing expected → skip
    if [[ ! -f "$expected" ]]; then
        echo "🟨 SKIP $label (missing $(basename "$expected"))"
        SKIP=$((SKIP + 1))
        return
    fi

    # Compare
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

    # Update mode
    if [[ $UPDATE -eq 1 ]]; then
        echo "$actual_code" > "$expected"
        echo "🔄 UPDATED: $label"
        PASS=$((PASS + 1))
        return
    fi

    # Missing expected → skip
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

    expected_out="./tests/$base.lox.out"
    expected_err="./tests/$base.lox.err"
    expected_exit="./tests/$base.lox.exit"

    actual_out="$TMP_DIR/$base.lox.out"
    actual_err="$TMP_DIR/$base.lox.err"

    # Run CLI once per input
    set +e
    "$BIN" "$file_in" >"$actual_out" 2>"$actual_err"
    exit_code=$?
    set -e

    # Treat each as independent tests
    run_test      "[out]  $base" "$expected_out" "$actual_out"
    run_test      "[err]  $base" "$expected_err" "$actual_err"
    run_exit_test "[exit] $base" "$expected_exit" "$exit_code"
done

echo
echo "Total: $TOTAL | Passed: $PASS | Failed: $FAIL | Skipped: $SKIP"

if [[ $FAIL -ne 0 ]]; then
    exit 1
fi