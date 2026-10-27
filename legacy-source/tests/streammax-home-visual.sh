#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 - <<'PY'
from pathlib import Path
import re

home = Path("src/home.c").read_text(encoding="utf-8")
layout = Path("src/layout.h").read_text(encoding="utf-8")
aj = Path("src/ajustes.c").read_text(encoding="utf-8")
up = Path("src/streammax_update.h").read_text(encoding="utf-8")

# O formato é uma escolha já existente: não modificar o índice persistido.
assert 'V_POSTER_FORMAT[] = { "Horizontais", "Verticais" }' in aj
assert 'ESC("Formato dos pôsteres",       V_POSTER_FORMAT, 2)' in aj
assert "return lig(AJ_LANDSCAPE);" in aj
assert "ajustes_posteres_deitados()" in home
assert "FILEIRA_CONTINUE" in home and "continuar_desenhar" in home
# O layout TV fica mais compacto sem mudar a geometria antiga do celular.
assert "#define NV_HOME_HERO_EMPURRA 144.0f" in layout
assert "return homeCinematicaTV() ? 378.0f : NV_SHELF_TOP;" in home
assert "return homeCinematicaTV() ? 48.0f : NV_LEGACY_ROW_HEAD_H;" in home
assert "if (homeCinematicaTV()) return 24.0f;" in home
assert "if (homeCinematicaTV()) return 36.0f;" in home
assert "if (t == FILEIRA_CONTINUE || t == FILEIRA_RETORNO) e *= 0.76f;" in home
assert "else if (t == FILEIRA_NORMAL || t == FILEIRA_TOP10_NUM) e *= 0.78f;" in home  # 1.0.7: +11.4%
assert "homeCabecalhoH()" in home
menu = Path("src/menu.c").read_text(encoding="utf-8")
assert "static int cinemaRail(void)" in menu
assert "logoapp_streammax_wordmark" in menu
assert "float e = cinemaRail() ? 1.0f : 0.0f;" in menu
assert "borda = 48.0f + 250.0f * NV_MENU_MODERNA_AUMENTO;" in aj
assert "(NV_SHELF_TOP + 481.0f) * 0.75f" in home
# Arte e trailer compartilham a caixa; proporcao da fonte nao a encolhe.
assert "sm_trailer_hero_area" not in home
assert "gfx_furo_hero_suave(furo, aTrailer, modoHero == GFX_HERO_CHEIO);" in home
assert "heroTrailerArea(heroArteRect)" in home
# O foco e o raio continuam animados e ajustáveis; acabamento do card mais fino.
assert "#define NV_HOME_ANEL_SUTIL 2.0f" in home
assert 'return ajustes_borda_foco() ? 0.02f : 0.045f;' in home
assert 'gfx_borda_foco_atual = ajustes_borda_foco() ? 0.55f : 0.0f;' in home
assert "anim_mola(animFoco[r][c], alvo, dt" in home
assert "ajustes_raio_poster_px()" in home
assert '#define SM_UPDATE_VERSION "1.0.22"' in up
print("OK: Home StreamMax 1.0.22: menu e fileiras da 1.0.4 preservados")
PY
