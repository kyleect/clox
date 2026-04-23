#!/usr/bin/env bash
set -euo pipefail

OUT="${1:-src/version.c}"

{
  echo "// This is a generated file. Do not edit!"
  echo "// Generated from VERSION.txt"
  echo
  echo "// Language Version: $(cat VERSION.txt)"
  echo

  xxd -i VERSION.txt | sed -e 's/};/, 0x00\n};/'
} > "$OUT"