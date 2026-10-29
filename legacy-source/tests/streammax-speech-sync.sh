#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
flags=(-std=gnu11 -O1 -g -Wall -Wextra -Wno-misleading-indentation -ffunction-sections -fdata-sections -Isrc)
if [ "${SM_SANITIZERS:-0}" = 1 ]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
for name in sync bridge; do
  ${CC:-cc} "${flags[@]}" "tests/test_streammax_speech_$name.c" src/legenda.c \
    src/legfala.c -Wl,--gc-sections -pthread -lm -o "$tmp/$name"
  "$tmp/$name"
done
${CC:-cc} "${flags[@]}" -DNV_TOUCH_UI -DNV_ANDROID \
  ${SM_CFLAGS:-$(pkg-config --cflags sdl2)} tests/test_streammax_speech_ui.c \
  src/legenda.c src/legfala.c -Wl,--gc-sections -pthread -lm -o "$tmp/ui"
"$tmp/ui"
