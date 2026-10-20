#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
  tests/test_streammax_tv_sidebar.c -o "$out"
"$out"
python3 - <<'PY'
from pathlib import Path
guia = Path("src/guia.c").read_text()
required = (
    "static void desenharStreammaxTv(float a, time_t agoraT)",
    "if (streammaxTvAtivo()) {\n    desenharStreammaxTv(a, agoraT);",
    "streammaxTvColuna = STREAMMAX_TV_FOCUS_CATEGORIES;",
    "streammaxTvColuna = STREAMMAX_TV_FOCUS_CHANNELS;",
    "streammax_tv_first_visible(focoLin, catsN, visible)",
    "streammax_tv_first_visible(focoCol, channelsN, visible)",
    "int now = gAgora((GCanal *)canal, agoraT, &p);",
    "int next = gProximo((GCanal *)canal, agoraT, 0, &q);",
    "if (streammaxTvAtivo() && aberta && !overlay)",
    "if (overlay) { desenharBanda(a, agora); return; }",
)
for marker in required:
    if marker not in guia:
        raise AssertionError(f"UI/EPG contract missing: {marker}")
assert "desenharLogo(" not in guia[guia.index("static void desenharStreammaxTv"):guia.index("void guia_desenhar(")]
print("OK: guia StreamMax em tres paineis, D-pad, EPG e sem logos")
PY
