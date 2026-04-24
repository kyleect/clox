#!/usr/bin/env bash
set -euo pipefail

NOW=$(date '+%Y-%m-%dT%T');

OUT="${1:-src/version.c}"

VERSION=$(tr -d '\n' < VERSION.txt)

LEN=$(wc -c < VERSION.txt)
NEW_LEN=$((LEN + 1))

{
  echo "// This is a generated file. Do not edit!"
  echo "// Generated from VERSION.txt"
  echo "// $NOW"
  echo
  echo "// clox $(cat VERSION.txt)"
  echo

  printf 'const char CLOX_VERSION[] = "%s";\n' "$VERSION"
  printf 'const unsigned int CLOX_VERSION_len = %d;\n' "${#VERSION}"
} > "$OUT"