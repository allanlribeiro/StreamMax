#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
cc -D_GNU_SOURCE -std=c11 -Wall -Wextra -Werror -Isrc src/addonmanifest.c src/js.c tests/test_streammax_addon_manifest.c -lm -o "$tmp/manifest"
"$tmp/manifest"
cc -D_GNU_SOURCE -std=c11 -Wall -Wextra -Werror -Isrc src/addoninst.c src/addonmanifest.c src/js.c tests/test_streammax_addon_install.c -pthread -lm -o "$tmp/install"
"$tmp/install"
