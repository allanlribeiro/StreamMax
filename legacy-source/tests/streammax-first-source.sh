#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
  tests/test_streammax_first_source.c -o "$out"
"$out"
# Guard the actual route, not only the standalone C decision function.
python3 - <<'PY'
from pathlib import Path
app = Path("src/app.c").read_text()
detail = Path("src/detail.c").read_text()
assert "streammax_should_ask_first_source(ajustes_fonte_manual(), stream_n(), lembrada)" in app
assert "streammax_should_ask_episode_source(" in detail
assert "tituloAtual && fontepref_tem(tituloAtual->imdb)" in detail
assert "pedFontesT = ep->temporada; pedFontesE = ep->episodio;" in detail
# Even if an old preference no longer matches new episode sources, the
# playback router must fall back to the picker instead of arbitrary autoplay.
assert "folhaParaTocar = 1;" in app
print("OK: detalhe e roteador VOD respeitam primeira fonte manual")
PY
echo "OK: regras de entrada VOD StreamMax validadas sem APK"
