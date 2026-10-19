#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
  tests/test_streammax_source_ptbr.c -o "$out"
"$out"
# O selo Dual Audio deve vir do addon, sem inferir faixas PT-BR e EN.
${CC:-cc} -std=c11 -Wall -Wextra -Werror -pedantic \
  tests/test_streammax_source_audio.c -o "$out"
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
assert '#include "streammax_source_audio.h"' in code
assert 'streammax_audio_dual_fonte(' in code
assert 'streammax_audio_dual_idiomas_fonte(' in parse
assert 'if (!s.audioIdiomas[0]) {' in parse
assert 'if (bits & SM_DUAL_BR)' in parse
assert 'else if (bits & SM_DUAL_PTPT)' in parse
assert 'else if (bits & SM_DUAL_PT)' in parse
assert 'if (bits & SM_DUAL_EN)' in parse
assert 'i18n("Áudio: idiomas não informados")' in code
assert 'if (!s->audioIdiomas[0] &&' in code
assert 'i18n("Idiomas não informados")' in code

assert 's->tituloFonte, s->descricao' in code
assert 'streammaxFonteDadosOriginais(' in code
# Nome/title/description completos precisam ocupar linhas independentes
# quando o addon envia varias linhas no proprio description.
assert '#define SM_FONTE_ORIG_LINHAS 12' in code
assert 'static int streammaxFonteLinhasOriginais(' in code
assert "while (*fim && *fim != '\\n' && *fim != '\\r') fim++;" in code
assert 'streammaxFonteLinhasOriginais(&lista[i], originais)' in code
assert 'streammaxFonteLinhasOriginais(s,originais)' in code
assert 'snprintf(d,sizeof d,"%.*s",(int)originais[mi].n,originais[mi].inicio)' in code
# A etiqueta Dual Audio nao pode sumir quando a linha tem muitos selos.
assert 'int dualReservado = 0;' in code
assert 'if(idiomaMarca && !strcmp(idiomaMarca, "Dual áudio"))' in code
# A CI anterior falhou porque o seletor usava estas variaveis ANTES da declaracao.
area = code.split('if(idiomaMarca && !strcmp(idiomaMarca, "Dual áudio"))', 1)[0]
assert area.rfind('const char *idiomaMarca = idiomaDa(s);') > area.rfind('// O IDIOMA')
assert area.rfind('int dualReservado = 0;') > area.rfind('// O IDIOMA')
assert 'tx0+=(float)du.w+22.0f;' in code
assert 'txtW0-=(float)du.w+22.0f;' in code
assert 'if(id && !dualReservado)' in code

assert 'FOLHA_ARQ_H * nOriginais' in code
assert 'txt_linha_corta(TXT_PG_FIM,d,155,168,187,255,tr-tx)' in code
assert 'char nomeFonteCompleto[1024];' in Path("src/streams.h").read_text()
assert 'char tituloFonte[1024];' in Path("src/streams.h").read_text()
assert 'snprintf(s.nomeFonteCompleto, sizeof s.nomeFonteCompleto' in parse
assert 'snprintf(s.tituloFonte, sizeof s.tituloFonte' in parse
assert 'streammax_audio_dual_fonte(s->arquivo, s->nomeFonteCompleto[0] ?' in code
assert 'if (podePt && (temNoTexto(s, "dublado") || temNoTexto(s, "dub")))' in code
assert 'if (podePt && temNoTexto(s, "dual"))' not in code


# Nao reutilizar a conversao de bandeiras na triagem da fonte.
assert '#include "streammax_flags.h"' in code
assert 'gfx_icone_colorido(' in code
assert 'streammax_flag_icone(raw)' in code
assert 'streammax_audio_bandeiras(s->audioIdiomas, bandeiras, 3)' in code
assert 'gfx_icone_colorido((GfxRect){bx, cy + (FOLHA_SELO_H - 28.0f)' in code
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
