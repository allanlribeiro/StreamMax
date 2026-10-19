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
# O ramo que abre a folha diretamente deve invalidar a busca auto ainda viva.
abrir = app.split("if (detail_pediu_fontes()) {", 1)[1].split("// Escolher uma fonte na folha inicia", 1)[0]
assert "if (!player_id_canal()[0] && aguardandoFonte != 0)" in abrir
assert "aguardandoFonte = 0;" in abrir
assert "fontePedidoGeracao = novaGeracaoFonte();" in abrir
assert "limparFontePendente();" in abrir
# Fim do prazo dos addons ou retorno do worker NAO pode ignorar a folha.
assert "streammax_auto_may_play(player_id_canal()[0] != 0," in app
assert app.count("streammax_auto_may_play(player_id_canal()[0] != 0,") >= 2
assert "stream_folha_aberta(), folhaParaTocar)" in app
# Apenas o OK numa fonte libera a trava manual; nao ha cronometro liberando.
escolha = app.split("if (aguardandoFonte != 2 && stream_folha_escolheu(&fonte)) {", 1)[1]
assert "folhaParaTocar = 0;" in escolha
print("OK: fonte nova nunca toca sozinha enquanto o seletor aguarda OK")
PY
echo "OK: regras de entrada VOD StreamMax validadas sem APK"
