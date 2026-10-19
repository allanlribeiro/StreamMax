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
assert "lembrada = base[0] ? fontepref_escolher_manual(base) : -1;" in app, "fonte automatica antiga nao pode pular a primeira escolha"
assert "int lembradaPreparo = basePreparo[0] ? fontepref_escolher_manual(basePreparo) : -1;" in app, "preparo nao pode usar preferencia automatica antiga"
assert "int usouPreparo = lembradaPreparo >= 0 &&" in app, "preparo sem preferencia manual nao pode ser adotado"
detail_preload = app.split("static void prepararAoAbrir(void) {", 1)[1].split("\n}\n", 1)[0]
assert "fontepref_escolher_manual(base)" in detail_preload, "pre-preparo usa somente fonte escolhida"
assert "if (lembrada < 0) { preparoInvalidar(); return; }" in detail_preload, "pre-preparo sem escolha manual deve ser cancelado"
partial = app.split("static int autoParcialPronto(void) {", 1)[1].split("\n}\n", 1)[0]
assert "const int manual = 1;" in partial, "busca parcial nao pode usar autoplay Legacy"
assert "fontepref_escolher_manual(base)" in partial, "busca parcial exige preferencia da pessoa"
assert "streammax_should_ask_episode_source(" in detail
assert "tituloAtual && fontepref_tem(tituloAtual->imdb)" in detail
assert "pedFontesT = ep->temporada; pedFontesE = ep->episodio;" in detail
# Even if an old preference no longer matches new episode sources, the
# playback router must fall back to the picker instead of arbitrary autoplay.
saved = app.split("static int tocarFonteGuardada(void) {", 1)[1].split("\n}\n", 1)[0]
assert "if (!base[0] || !fontepref_tem(base)) return 0;" in saved
assert "if (fontepref_escolher_lista(base, &s, 1, NULL, NULL) != 0)" in saved
assert "ajustes_fonte_manual()" not in saved, "fonte guardada nunca pula escolha por ajuste herdado"
assert "folhaParaTocar = 1;" in app
print("OK: detalhe e roteador VOD respeitam primeira fonte manual")
PY
echo "OK: regras de entrada VOD StreamMax validadas sem APK"
