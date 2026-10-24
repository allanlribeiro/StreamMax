#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
cc -std=c11 -Wall -Wextra -Werror -Isrc src/trailerhls.c tests/test_streammax_trailers.c -lm -o "$tmp/trailers"
"$tmp/trailers" "${@}"
python3 - <<'CHECK'
from pathlib import Path
home = Path("src/home.c").read_text()
assert "sm_trailer_hero_area(NV_TELA_W" in home
assert "GfxRect furo = heroTrailerArea(r);" in home
assert "heroTrailerArea(heroArteRect)" in home
assert "gfx_furo_hero_suave(furo, aTrailer, SM_TRAILER_FADE_ESQUERDA, SM_TRAILER_FADE_BASE);" in home
assert "destCartao ? cartaoVideoRect(cartaoTrailerRect) : heroTrailerArea(heroArteRect)" in home
trailer = Path("src/trailer.c").read_text()
assert "dono == TRAILER_DONO_HOME && !noCartao && !cheia" in trailer
assert "if (!heroAndroid && video_recorte_fonte_trailer())" in trailer
menu = Path("src/menu.c").read_text()
assert "case MENU_PERFIL:   return 0;" in menu
print("OK: geometria compartilhada, cartoes/detalhes preservados, Perfil e Stats oculto")
CHECK
