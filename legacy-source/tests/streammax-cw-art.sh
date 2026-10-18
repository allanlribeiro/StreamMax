#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
# So as funcoes utilizadas no teste entram no link, sem dependencias SDL/GL.
cc -std=c11 -O1 -D_DEFAULT_SOURCE -ffunction-sections -fdata-sections \
  -Isrc tests/streammax-cw-art.c src/artehero.c \
  -Wl,--gc-sections -o "$out"
"$out"
