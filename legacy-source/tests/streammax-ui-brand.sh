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
