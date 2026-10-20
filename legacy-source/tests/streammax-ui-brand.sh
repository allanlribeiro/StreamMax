#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bin="$(mktemp)"
trap 'rm -f "$bin"' EXIT
cc -std=c11 -Wall -Wextra -Werror -I src \
  tests/test_streammax_ui_brand.c -o "$bin"
"$bin"

# Regressao visual da escolha de perfis: as fotos, cartoes de continuar e PIN
# devem permanecer intactos, enquanto a marca e o foco usam azul cinematografico.
python3 - <<'PY'
from pathlib import Path
src = Path("src/perfilsel.c").read_text(encoding="utf-8")
assert 'static void streammaxPerfilAcabamento(float a)' in src
assert 'streammaxPerfilAcabamento(a * (pinDe >= 0 ? 0.60f : 1.0f));' in src
assert 'txt_linha(TXT_G30B, "STREAM"' in src
assert 'txt_linha(TXT_G30B, "MAX"' in src
assert '"ESCOLHA DE PERFIL"' in src
assert 'muralLuzAlvo[0] = 0.045f;' in src
assert 'cr = 0.035f; cg = 0.55f; cb = 1.0f;' in src
assert '0.18f, 0.70f, 1.0f, f * a' in src
assert 'psestilos_desenhar(&cena, modo,' in src
assert 'disco(av, p, f, a, i == foco && pinDe < 0);' in src
assert 'contDesenhar(i,' in src
assert 'if (animPin > 0.004f) desenhaPin();' in src
assert 'foco = perfis_indice_sugerido();' in src
print("OK: StreamMax Signature nos perfis, com fotos, PIN e Continuar intactos")
PY

# Contrato visual do player: audio, legendas e fontes compartilham vidro fume,
# sem alterar a escolha manual, o Media3 ou a navegacao pelo controle remoto.
python3 - <<'PY'
from pathlib import Path
ilha = Path("src/plrilha.c").read_text(encoding="utf-8")
leg = Path("src/legendasui.c").read_text(encoding="utf-8")
fontes = Path("src/streams.c").read_text(encoding="utf-8")
faixas = Path("src/faixas.c").read_text(encoding="utf-8")
assert 'if (c->painelEsquerdo) {' in ilha
assert 'gfx_cor(R, raio, 0.015f, 0.022f, 0.038f, 0.78f * cenA);' in ilha
assert 'const int seletorLegendas = c->w > 1200.0f;' in ilha
assert 'desenharIdiomas(x0, y, LU_IDIOMAS_LARG, a);' in leg
assert 'desenharEstiloResumo(' in leg
assert 'float sep1 = ' in leg and 'float sep2 = ' in leg
assert 'gfx_cor(corpo,FOLHA_RAIO_IL/corpo.h,.015f,.022f,.038f,.78f*anim);' in fontes
assert 'stream_folha_escolheu' in fontes
assert 'else if (modo) legendasui_corpo(c, a);' in faixas
assert 'else corpoLista(c, a);' in faixas
print("OK: identidade visual comum em audio, legendas e fontes")
PY
