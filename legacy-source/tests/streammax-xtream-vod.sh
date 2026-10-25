#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
${CC:-cc} -std=gnu11 -O1 -g -Wall -Wextra -Werror -Wno-format-truncation \
  -ffunction-sections -fdata-sections -Isrc src/xtream.c src/js.c \
  tests/test_streammax_xtream_vod.c -Wl,--gc-sections -pthread -lm -o "$tmp/vod"
"$tmp/vod"
