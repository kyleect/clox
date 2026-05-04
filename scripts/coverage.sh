#!/usr/bin/env bash

set -euo pipefail

ENABLE_COVERAGE=ON ./scripts/build.sh

./scripts/tests.sh

lcov --capture --directory . --output-file lcov.info 
