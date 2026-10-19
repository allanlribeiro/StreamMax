#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
  tests/test_streammax_source_ptbr.c -o "$out"
"$out"
# Bandeiras devem ser so uma apresentacao visual das faixas declaradas.
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
  tests/test_streammax_flags.c -o "$out"
"$out"
python3 - <<'PY'
from pathlib import Path
code = Path("src/streams.c").read_text()
parse = Path("src/stream_parse.c").read_text()
section = code.split("static int temAudioPreferido(const Stream *s) {", 1)[1].split("static const char *ajudaFiltroAudio", 1)[0]
assert 'if (!pref[0] || audioDeclaradoEm(s, pref)) return 1;' in section
assert 'if (s->audioIdiomas[0]) return 0;' in section
assert 'return ling_casa("pt", pref) &&' in section
assert 'streammax_pt_audio_sem_declaracao(s->arquivo, s->rotulo, s->descricao)' in section
assert '#include "streammax_source_ptbr.h"' in code
# Nao reutilizar a conversao de bandeiras na triagem da fonte.
assert '#include "streammax_flags.h"' in code
assert 'gfx_icone_colorido(' in code
assert 'streammax_flag_icone(raw)' in code
assert 'streammax_flag_icone(' not in section
assert 'streammax_flag_idioma_legivel(raw)' in code
assert "d[fimTexto - 1] == '/'" in code
from pathlib import Path
for iso in ('br', 'pt', 'us', 'gb', 'es', 'mx'):
    caminho = Path("deploy/app/art/icones") / f"sm_flag_{iso}.png"
    assert caminho.is_file(), caminho
    assert caminho.read_bytes()[:8] == bytes((137, 80, 78, 71, 13, 10, 26, 10))
for name in ('Português brasileiro', 'Portugues brasileiro', 'Português do Brasil', 'Portuguese Brazilian'):
    assert name in parse and name in code, name
print("OK: bandeiras so na interface; PT-BR permanece filtrado pelo audio declarado")
PY
