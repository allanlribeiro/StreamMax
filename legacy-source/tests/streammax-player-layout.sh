#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bin="$(mktemp)"
trap 'rm -f "$bin"' EXIT
cc -std=c11 -Wall -Wextra -Werror -Isrc tests/test_streammax_player_layout.c -o "$bin"
"$bin"
