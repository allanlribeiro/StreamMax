#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
${CC:-cc} -std=gnu11 -O1 -g -Wall -Wextra -Werror -Wno-format-truncation \
  ${SM_SANITIZERS:+-fsanitize=address,undefined -fno-omit-frame-pointer} \
  -ffunction-sections -fdata-sections -Isrc src/minifonte.c src/seekrvtt.c src/js.c \
  tests/test_minifonte.c -Wl,--gc-sections -lm -o "$tmp/parser"
"$tmp/parser"
if [[ ${SM_PLAYER_TESTS:-0} == 1 ]]; then
  # CI instala libsdl2-dev/libsdl2-image-dev/libgles2-mesa-dev. Variaveis abaixo
  # permitem usar um sysroot local sem instalar pacotes no computador.
  ${CC:-cc} -std=gnu11 -O1 -g -Wall -Wextra -Wno-format-truncation \
    ${SM_SANITIZERS:+-fsanitize=address,undefined -fno-omit-frame-pointer} \
    -ffunction-sections -fdata-sections -Isrc ${SM_THUMB_CFLAGS:-$(pkg-config --cflags sdl2 SDL2_image)} \
    tests/test_minifonte_player.c src/minifonte.c src/minifonte_rede.c \
    src/rede.c src/redeurl.c src/seekrvtt.c src/js.c src/jpegrapido.c \
    -Wl,--gc-sections ${SM_THUMB_LIBS:-$(pkg-config --libs sdl2 SDL2_image)} \
    -ldl -pthread -lm -o "$tmp/player"
  python3 tests/minifonte_fixture.py "$tmp/player"
fi
