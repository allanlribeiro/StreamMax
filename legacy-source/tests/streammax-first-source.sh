#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
  tests/test_streammax_first_source.c -o "$out"
"$out"
echo "OK: regras de entrada VOD StreamMax validadas sem APK"
