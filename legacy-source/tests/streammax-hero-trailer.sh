#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 - <<'PY'
from pathlib import Path
import re
home = Path("src/home.c").read_text()
layout = Path("src/layout.h").read_text()
def define(key):
    match = re.search(r"^#define\s+"+re.escape(key)+r"\s+([0-9]+(?:\.[0-9]+)?)f?",layout,re.M)
    assert match, key
    return float(match.group(1))
# A imagem pode ser tela cheia; o video usa um destino proprio e termina
# ANTES do viewport onde as fileiras ficam visiveis (TCL/Android TV).
slot_bottom = min(define("NV_HERO_ARTE_H"), define("NV_SHELF_TOP") + 40.0)
assert slot_bottom > 0 and slot_bottom < define("NV_TELA_BASE_H")
assert define("NV_HERO_ARTE_X") < define("NV_TELA_BASE_W")
assert "static GfxRect heroTrailerArea(GfxRect arte)" in home
assert "float h = fminf(NV_HERO_ARTE_H, NV_SHELF_TOP + 40.0f);" in home
assert slot_bottom >= 550.0
# Na TCL a imagem deve tocar a borda DIREITA: o quadro nativo nao pode
# continuar alinhado ao centro com tarja preta no lado direito.
assert "float w = fminf(NV_HERO_ARTE_W, NV_TELA_W * 0.64f);" in home
assert "float x = NV_TELA_W - w;" in home
assert "return (GfxRect){x, 0.0f, w, h};" in home
assert 0.60 <= 0.64 <= 0.70

assert "NV_DIN_REPOUSO_FIL - 120.0f" in home
# O destino do SurfaceView e o furo do canvas devem ser IDENTICOS.
assert "GfxRect furo = heroTrailerArea(r);" in home
assert "heroTrailerArea(heroArteRect)" in home
assert "gfx_furo_alfa(furo, aTrailer);" in home
assert "gfx_furo_hero_suave(furo, aTrailer, 0.40f, 0.75f);" in home
assert "else if (aTrailer > 0.0f) gfx_furo_alfa(furo, aTrailer);" in home
gfx = Path("src/gfx.c").read_text()
gfx_h = Path("src/gfx.h").read_text()
assert "GFX_FURO_HERO = 45" in gfx_h
assert "GFX_NMODOS = 46" in gfx_h
assert "void gfx_furo_hero_suave(" in gfx
assert "glBlendFuncSeparate(GL_ZERO, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA);" in gfx
assert "float e = smoothstep(0.0, max(uPar.x,0.02), vUv.x);" in gfx
assert "float t = smoothstep(0.0, 0.085, vUv.y);" in gfx
assert "float b = 1.0-smoothstep(uPar.y, 1.0, vUv.y);" in gfx
assert "float abertura = clamp(uFoco,0.0,1.0) * e * t * b;" in gfx
assert "1.0-abertura" in gfx

assert "gfx_rect(furo, 0, GFX_HERO, 0, 1.0f, 0, 0.0f, 0, 0, 0, aTrailer);" in home
# A arte do destaque deve sumir progressivamente no Android,
# sem afetar o texto e as fileiras.
assert "if (aTrailer > 0.0f && layoutHome() == HOME_LAYOUT_MODERNA &&" in home
assert "aArte *= (1.0f - aTrailer);" in home.split("// Dissolver o backdrop quando o trailer assume o destaque.",1)[1].split("#endif",1)[0]
trailer = Path("src/trailer.c").read_text()
assert "dono == TRAILER_DONO_HOME && !noCartao && !cheia" in trailer
assert "if (!heroAndroid && recortePendente" in trailer
assert "(heroAndroid || (tocandoDesde && SDL_GetTicks() - tocandoDesde >= 800))" in trailer
assert "video_janela_fonte(sx, sy, sw, sh," in trailer

# O Android preserva o backdrop fora do furo, em vez de apagar a tela inteira.
first = home.split("// No Android a arte de fundo continua opaca fora do hero.",1)[1]
assert first.split("#endif",1)[0].count("aArte *= (1.0f - aTrailer);") == 1
assert "#ifndef NV_ANDROID" in first.split("if (deslizando)",1)[0]
# "No card" nao deve ser alterado pelo novo destino do hero.
assert "destCartao ? cartaoVideoRect(cartaoTrailerRect) : heroTrailerArea(heroArteRect)" in home
# O trailer nao ativa o modo cinema de tela cheia na TCL.
assert "&& 0" in home.split("// TCL: trailer permanece no hero",1)[1].split("#endif",1)[0]
print(f"OK: trailer direito 558 px, degradê inferior começa em 75%, esmaecimento alfa preservado (h={slot_bottom:.0f})")
PY
