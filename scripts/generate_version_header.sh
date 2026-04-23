#!/usr/bin/env bash

xxd -i VERSION.txt | sed 's/\([0-9a-f]\)$/\0, 0x00/' > src/version.h
echo -e "// This is a generated file. Do not edit!\n// Generated from VERSION.txt\n\n// Language Version: $(cat VERSION.txt)\n$(cat src/version.h)" > src/version.h
