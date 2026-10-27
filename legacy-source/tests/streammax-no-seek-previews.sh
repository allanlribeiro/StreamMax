#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 - <<'PY'
from pathlib import Path
s=Path('src/player.c').read_text()
assert 'seekr' not in s.lower(), 'Player ainda depende de miniaturas'
assert not list(Path('src').glob('minifonte*')), 'Carregador de frames ainda presente'
assert 'miniaturasVtt' not in Path('src/streams.h').read_text()
assert 'minifonte' not in Path('src/stream_parse.c').read_text()
assert 'AJ_SEEKR_' not in Path('src/ajustes_ux_tela.inc').read_text()
assert 'f-seekr' not in Path('src/ajustes_ux_guia_dados.inc').read_text()
print('OK: player sem consultas, recortes, texturas ou painel de miniaturas')
PY
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
${CC:-cc} -std=gnu11 -O1 -g -ffunction-sections -fdata-sections \
  -Isrc ${SM_CFLAGS:-$(pkg-config --cflags sdl2 SDL2_image SDL2_ttf)} \
  tests/test_streammax_no_seek_previews.c -Wl,--gc-sections -lm -o "$tmp/no-previews"
"$tmp/no-previews"
