#!/usr/bin/env bash
# Bootstrap de uma árvore separada do StreamMax Legacy, sem tocar no app Kotlin.
set -euo pipefail

UPSTREAM_URL="https://github.com/iqui27/nuvio-native-legacy.git"
DEST="${1:-../StreamMax-Legacy}"
if [[ -e "$DEST" ]]; then
  echo "Destino já existe: $DEST (não sobrescrevo)" >&2
  exit 1
fi

git clone --depth 1 "$UPSTREAM_URL" "$DEST"
python3 - "$DEST" <<'PY'
from pathlib import Path
import sys
root = Path(sys.argv[1])
gradle = root / "android/app/build.gradle.kts"
text = gradle.read_text()
old = 'applicationId = "space.nuvio.nativelegacy"'
assert text.count(old) == 1, "ID do Legacy mudou; revisar antes de adaptar"
gradle.write_text(text.replace(old, 'applicationId = "com.streammax.legacy"'))

strings = root / "android/app/src/main/res/values/strings.xml"
text = strings.read_text()
old = '<string name="app_name">Nuvio Legacy</string>'
assert text.count(old) == 1, "Recurso do nome mudou"
strings.write_text(text.replace(old, '<string name="app_name">StreamMax</string>'))
PY
echo "Base Legacy preparada em $DEST"
echo "Próximos passos: revisar aliases/identidade, idioma inicial no núcleo C, IPTV e dependências nativas."
echo "Não distribua um APK antes de testar a instalação, deep links, QR login e IPTV."
