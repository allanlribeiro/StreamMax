#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
  tests/test_streammax_next_prefetch.c -o "$out"
"$out"
echo "OK: prefetch da lista do proximo episodio preserva escolha manual"
