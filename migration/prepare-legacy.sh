#!/usr/bin/env bash
# Bootstrap de uma árvore separada do StreamMax Legacy, sem tocar no app Kotlin.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UPSTREAM_URL="https://github.com/iqui27/nuvio-native-legacy.git"
# Revisão fixada para evitar que mudanças futuras do Legacy quebrem a migração.
UPSTREAM_COMMIT="549e2273ce898e514cefd60fa2cbbbb53643c199"
DEST="${1:-../StreamMax-Legacy}"
if [[ -e "$DEST" ]]; then
  echo "Destino já existe: $DEST (não sobrescrevo)" >&2
  exit 1
fi

git clone --no-checkout "$UPSTREAM_URL" "$DEST"
git -C "$DEST" checkout --detach "$UPSTREAM_COMMIT"
ACTUAL_COMMIT="$(git -C "$DEST" rev-parse HEAD)"
[[ "$ACTUAL_COMMIT" == "$UPSTREAM_COMMIT" ]] || { echo "Revisão inesperada do Legacy" >&2; exit 1; }
python3 - "$DEST" <<'PY'
from pathlib import Path
import sys
root = Path(sys.argv[1])
gradle = root / "android/app/build.gradle.kts"
text = gradle.read_text()
old = 'applicationId = "space.nuvio.nativelegacy"'
assert text.count(old) == 1, "ID do Legacy mudou; revisar antes de adaptar"
gradle.write_text(text.replace(old, 'applicationId = "com.streammax.tv"'))

strings = root / "android/app/src/main/res/values/strings.xml"
text = strings.read_text()
old = '<string name="app_name">Nuvio Legacy</string>'
assert text.count(old) == 1, "Recurso do nome mudou"
strings.write_text(text.replace(old, '<string name="app_name">StreamMax</string>'))

# Na falta de uma preferência detectável, o idioma automático usa pt-BR.
# A escolha manual gravada no Legacy continua tendo prioridade, pois
# idiomaResolver() já retorna se valor[AJ_IDIOMA] != 0.
ajustes = root / "src/ajustes.c"
text = ajustes.read_text()
initial = 'static int idiomaEfetivo = IDIOMA_EN;'
resolver = 'novo = idiomaauto_resolver(contaTmdbLing, contaLegLing, sistemaLoc, &fonte);'
assert text.count(initial) == 1, "Inicialização de idioma alterada no upstream"
assert text.count(resolver) == 1, "Resolução automática de idioma alterada no upstream"
text = text.replace(initial, 'static int idiomaEfetivo = IDIOMA_PT;')
text = text.replace(resolver, resolver + '\n  if (fonte == IDA_PADRAO) novo = IDIOMA_PT; /* StreamMax: fallback pt-BR */')
first_run = "    // Nunca gravou nada: o idioma nasce automatico (o padrao de valor[]).\\n    valor[AJ_IDIOMA] = 0;"
assert text.count(first_run) == 1, "Regra da primeira instalação mudou no Legacy"
text = text.replace(first_run, "    // StreamMax: primeira instalação inicia em português brasileiro.\\n    // Preferência manual posterior continua salva e respeitada.\\n    valor[AJ_IDIOMA] = IDIOMA_PT + 1;")
ajustes.write_text(text)

# TV ao vivo: validar que a fonte continua fornecendo os módulos nativos
# responsáveis pelo cadastro, guia e resolução das fontes de IPTV.
for module in ("src/xtream.c", "src/xtream.h", "src/stalker.c", "src/stalker.h", "src/guia.c", "src/login.c", "src/sessao.c"):
    assert (root / module).is_file(), f"Modulo IPTV ausente: {module}"

# Autenticação: o Legacy já oferece QR e login por e-mail/senha.
# Proteger os dois fluxos de uma remoção acidental durante a migração.
login = (root / "src/login.c").read_text()
assert "sessao_login_email(email, senha)" in login, "Login por e-mail/senha removido"
assert "sessao_login_comecar()" in login, "Login QR removido"

# Conferência estática antes de qualquer build Android.
assert 'if (valor[AJ_IDIOMA] != 0) return;' in text
assert 'if (fonte == IDA_PADRAO) novo = IDIOMA_PT;' in text
PY
python3 "$SCRIPT_DIR/check-legacy-auth.py" "$DEST"
echo "Base Legacy preparada em $DEST"
echo "Próximos passos: incorporar o Legacy ao repositório, revisar identidade, player, idioma e dependências nativas."
echo "Não distribua um APK antes de testar a instalação, deep links, QR login e IPTV."
