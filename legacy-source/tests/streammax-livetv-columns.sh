#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
  tests/test_streammax_livetv_columns.c -o "$out"
"$out"
python3 - <<'PY'
from pathlib import Path
src = Path("src/guia.c").read_text(encoding="utf-8")
assert 'static int tvTresColunas = 1;' in src
assert 'static void streammaxTvDesenhar(float a, time_t agoraT)' in src
assert 'streammaxTvDesenhar(a, agoraT);' in src
assert 'tvColunaFoco == SM_TV_COL_CATEGORIAS' in src
assert 'tvColunaFoco = SM_TV_COL_CANAIS;' in src
assert 'focoLin = streammax_tv_limitar(focoLin + d, nLinhas());' in src
assert 'focoCol = streammax_tv_limitar(focoCol + d, linhaN(focoLin));' in src
assert 'else if (!okDesde) { okDesde = agora; okLongo = 0; }' in src
assert '!tvTresColunas && previewLigado && !previewPreso' in src
assert 'if (tvTresColunas && !gRetrato()) {' in src
section = src.split('static void streammaxTvDesenhar(', 1)[1].split('void guia_desenhar(', 1)[0]
assert 'gAgora(canal, agoraT, &ag)' in section
assert 'gProximo(canal, agoraT, 0, &prox)' in section
assert 'streammax_tv_progresso(' in section
assert 'guia_logo_desenhar(' not in section and 'logoNaCaixa(' not in section
assert 'pedirCanal(' not in section
assert 'gfx_furo_raio(pv, raio);' in section
assert 'video_pronto() && !player_carregando()' in section
assert 'guia_selo_ao_vivo(' in section
assert '(previewLigado || tvTresColunas)' in src
assert 'tvColunaFoco == SM_TV_COL_CANAIS' in src
assert 'previewFocoMudou = agora;' in src
assert 'tvTresColunas = 0; modoLista = 1' in src
assert 'tvTresColunas = 1; tvColunaFoco = SM_TV_COL_CANAIS' in src
assert '*y = SM_TV_BODY_Y + 84.0f;' in src
print("OK: tela de TV 3 colunas, EPG real e previa de video com furo apos primeiro quadro")
PY
