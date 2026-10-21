#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 - <<'PY'
from pathlib import Path
ctx = Path("src/ctxmenu.c").read_text(encoding="utf-8")
head = Path("src/ctxmenu.h").read_text(encoding="utf-8")
app = Path("src/app.c").read_text(encoding="utf-8")
home = Path("src/home.c").read_text(encoding="utf-8")
rows = Path("src/fileiras.c").read_text(encoding="utf-8")

# A click must expose a real modal, not just close the context menu.
assert 'juntar("Renomear fileira", OP_RENOMEAR_FILEIRA);' in ctx
assert 'teclado_abrir_com("Renomear fileira"' in ctx
assert 'int ctx_renomear_ativo(void)' in ctx
assert 'int  ctx_renomear_ativo(void);' in head

# The Home, unlike Settings, is responsible for all THREE keyboard paths.
assert 'if (ctx_renomear_ativo()) { teclado_evento(e); return; }' in app
assert 'if (ctx_renomear_ativo()) teclado_atualizar(dt, agora);' in ctx
assert 'if (ctx_renomear_ativo()) {' in app
assert 'teclado_desenhar(agora);' in app
assert app.index('if (ctx_renomear_ativo()) { teclado_evento(e); return; }') < app.index('if (ctx_aberto())     { ctx_evento(e);')
assert app.index('teclado_desenhar(agora);') > app.index('ctx_desenhar(agora);')
assert ctx.index('teclado_atualizar(dt, agora);') < ctx.index('renomearLerTeclado();', ctx.index('void ctx_atualizar'))

# Result is applied only after successful completion, to the same profile.
assert 'resultado == TECLADO_PRONTO && nv_perfil_sessao_atual(renomearSessao)' in ctx
assert 'fil_definir_apelido(renomearPendente, teclado_texto());' in ctx
assert 'renomearPendente[0] = 0' in ctx

# Labels visible on the Home are overlaid after registering canonical titles.
assert 'int fil_copiar_apelido(' in rows
assert 'fil_copiar_apelido(fileiras[ix].chave, apelido, sizeof apelido)' in home
assert home.index('fil_espelhar_ordem(ch, ti, destino);') < home.index('fil_copiar_apelido(fileiras[ix].chave, apelido, sizeof apelido)')
assert home.index('fil_copiar_apelido(fileiras[ix].chave, apelido, sizeof apelido)') < home.index('q = fil_unir(ch, destino, ord, MAX_FIL);')

print("OK: teclado da Home roteado, atualizado, desenhado e apelido preservado por perfil")
PY
