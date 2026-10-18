#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bin="$(mktemp)"
trap 'rm -f "$bin"' EXIT
cc -std=c11 -Wall -Wextra -Werror -I src \
  tests/test_streammax_ui_brand.c -o "$bin"
"$bin"
