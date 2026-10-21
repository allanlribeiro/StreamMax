#!/usr/bin/env bash
# StreamMax Windows x64 bootstrap build under MSYS2/UCRT64.
# W0 target: real native window/UI. Video is intentionally still the desktop stub.
set -euo pipefail
cd "$(dirname "$0")/.."

BUILD_ONLY=0
PREVIEW=0
while [ "$#" -gt 0 ]; do
  case "$1" in
    --build) BUILD_ONLY=1; shift ;;
    --preview) PREVIEW=1; export NUVIO_UI_PREVIEW=1; shift ;;
    *) break ;;
  esac
done

CC="${CC:-gcc}"
DEPS=(sdl2 SDL2_image SDL2_ttf angleproject zlib)
if ! pkg-config --exists "${DEPS[@]}"; then
  echo "windows.sh: faltam SDL2, SDL2_image, SDL2_ttf, ANGLE e zlib no MSYS2 UCRT64." >&2
  exit 2
fi

ENVF="$(mktemp)"
trap 'rm -f "$ENVF"' EXIT
bash tools/env.sh --env-file "$ENVF"
DEFINES=(-DNV_WINDOWS_DESKTOP -Iwindows-compat -include windows_compat.h)
while IFS='=' read -r name value; do
  value=${value//\\/\\\\}
  value=${value//\"/\\\"}
  DEFINES+=("-D$name=\"$value\"")
done < "$ENVF"

read -r -a DEP_CFLAGS <<< "$(pkg-config --cflags "${DEPS[@]}")"
read -r -a DEP_LIBS <<< "$(pkg-config --libs "${DEPS[@]}")"

mkdir -p build/windows
"$CC" src/*.c src/dts/*.c -Isrc -o build/windows/StreamMax.exe -O1 -g   "${DEFINES[@]}" "${DEP_CFLAGS[@]}" "${DEP_LIBS[@]}"   -lws2_32 -lole32 -lshell32 -pthread -lm

echo "Windows UI build: $PWD/build/windows/StreamMax.exe"
if [ "$BUILD_ONLY" = 1 ]; then exit 0; fi

if [ -n "${LOCALAPPDATA:-}" ]; then
  DEFAULT_DATA="$LOCALAPPDATA/StreamMax"
else
  DEFAULT_DATA="$HOME/AppData/Local/StreamMax"
fi
if [ "$PREVIEW" = 1 ]; then DEFAULT_DATA="$DEFAULT_DATA-preview"; fi
export NUVIO_DADOS="${NUVIO_DADOS:-$DEFAULT_DATA}"
mkdir -p "$NUVIO_DADOS"
rm -f "$ENVF"
trap - EXIT
exec build/windows/StreamMax.exe "$PWD/deploy/app/art" "$@"
