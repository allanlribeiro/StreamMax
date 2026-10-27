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
area = home[home.index("static GfxRect heroTrailerArea("):home.index("static GfxRect heroArtworkRect(")]
assert "video_largura" not in area and "video_altura" not in area
assert "return arte;" in area
assert "GfxRect furo = heroTrailerArea(r);" in home
assert "heroTrailerArea(heroArteRect)" in home
assert "gfx_furo_hero_suave(furo, anim_suave(aTrailer), modoHero == GFX_HERO_CHEIO);" in home
assert "if (!trailer_aberto() || trailer_dono() != TRAILER_DONO_HOME || !trailer_mostra_video())" in home
assert "destCartao ? cartaoVideoRect(cartaoTrailerRect) : heroTrailerArea(heroArteRect)" in home
trailer = Path("src/trailer.c").read_text()
assert "dono == TRAILER_DONO_HOME && !noCartao && !cheia" in trailer
assert "if (!heroAndroid && video_recorte_fonte_trailer())" in trailer
assert "return aberto && video_quadro_visto();" in trailer
menu = Path("src/menu.c").read_text()
assert "case MENU_PERFIL:   return 0;" in menu
print("OK: geometria compartilhada, cartoes/detalhes preservados, Perfil e Stats oculto")
CHECK
