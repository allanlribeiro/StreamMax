#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/tests
sources=()
for f in src/*.c; do
  if [[ "$f" != src/main.c ]]; then sources+=("$f"); fi
done
# Reuse desktop video stubs; the test includes main.c under an unused entry name.
cc -DNV_LINUX_DESKTOP -std=gnu11 -O0 -g \
  -ffunction-sections -fdata-sections -Wl,--gc-sections \
  $(pkg-config --cflags sdl2 SDL2_image SDL2_ttf glesv2 egl zlib) \
  -Isrc tests/test_streammax_track_panels.c "${sources[@]}" src/dts/*.c \
  -o build/tests/track-panels \
  $(pkg-config --libs sdl2 SDL2_image SDL2_ttf glesv2 egl zlib) -ldl -pthread -lm
NUVIO_DADOS="$(mktemp -d)" build/tests/track-panels
