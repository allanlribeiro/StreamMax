#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
  tests/test_streammax_source_ptbr.c -o "$out"
"$out"
python3 - <<'PY'
from pathlib import Path
code = Path("src/streams.c").read_text()
parse = Path("src/stream_parse.c").read_text()
section = code.split("static int temAudioPreferido(const Stream *s) {", 1)[1].split("static const char *ajudaFiltroAudio", 1)[0]
assert 'if (soDub == 0) return 1;' in section
assert 'if (soDub == 3)' in section
assert 'return audioDeclaradoDual(s);' in section
assert 'static int audioDeclaradoDual(const Stream *s)' in code
assert '"pt", "en", "es", "fr", "de", "it", "ja", "ko", "ru", "zh", "hi"' in code
assert 'if (!pref[0] || audioDeclaradoEm(s, pref)) return 1;' in section
assert 'soDub = (soDub + 1) % 4;' in code
assert 'soDub == 1 ? "pt" : soDub == 2 ? "en" : ""' in code
assert 'Áudio: %s' in code
visual = code.split("static const char *idiomaDa(const Stream *s) {", 1)[1].split("// Dual Audio exige", 1)[0]
assert 'return "Português";' in visual, "texto isolado PT nao comprova audio duplo"
assert 'temNoTexto(s, "pt-br")' in visual, "manter identificação textual PT"

assert 'if (s->audioIdiomas[0]) return 0;' in section
assert 'if (ling_casa("pt", pref))' in section
assert 'if (ling_casa("en", pref))' in section
assert 'streammax_en_audio_sem_declaracao(s->arquivo, s->rotulo, s->descricao)' in section
assert 'streammax_pt_audio_sem_declaracao(s->arquivo, s->rotulo, s->descricao)' in section
assert '#include "streammax_source_ptbr.h"' in code
for name in ('Português brasileiro', 'Portugues brasileiro', 'Português do Brasil', 'Portuguese Brazilian'):
    assert name in parse and name in code, name
print("OK: filtragem PT-BR aplicada apenas ao chip de fontes, com audio declarado prioritario")
PY
