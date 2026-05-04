#!/usr/bin/env bash

set -euo pipefail

./build/clox -f examples/$1.lox -- ${@:2}