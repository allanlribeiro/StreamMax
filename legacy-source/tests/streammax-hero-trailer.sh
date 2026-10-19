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
slot_bottom = min(define("NV_HERO_ARTE_H"), define("NV_SHELF_TOP") - 96.0)
assert slot_bottom > 0 and slot_bottom < define("NV_TELA_BASE_H")
assert define("NV_HERO_ARTE_X") < define("NV_TELA_BASE_W")
assert "static GfxRect heroTrailerArea(GfxRect arte)" in home
assert "float h = fminf(NV_HERO_ARTE_H, corteFileiras());" in home
assert "NV_DIN_REPOUSO_FIL - 120.0f" in home
# O destino do SurfaceView e o furo do canvas devem ser IDENTICOS.
assert "GfxRect furo = heroTrailerArea(r);" in home
assert "heroTrailerArea(heroArteRect)" in home
assert "gfx_furo_alfa(furo, aTrailer);" in home
# O Android preserva o backdrop fora do furo, em vez de apagar a tela inteira.
first = home.split("// No Android a arte de fundo continua opaca fora do hero.",1)[1]
assert first.split("#endif",1)[0].count("aArte *= (1.0f - aTrailer);") == 1
assert "#ifndef NV_ANDROID" in first.split("if (deslizando)",1)[0]
# "No card" nao deve ser alterado pelo novo destino do hero.
assert "destCartao ? cartaoVideoRect(cartaoTrailerRect) : heroTrailerArea(heroArteRect)" in home
# O trailer nao ativa o modo cinema de tela cheia na TCL.
assert "&& 0" in home.split("// TCL: trailer permanece no hero",1)[1].split("#endif",1)[0]
print(f"OK: preview do hero limitado antes das fileiras ({slot_bottom:.0f}px na Moderna), autoplay preservado")
PY
