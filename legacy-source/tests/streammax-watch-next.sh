#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
javac -d "$tmp" android/app/src/main/java/space/nuvio/nativelegacy/WatchNextPolicy.java tests/TestWatchNextPolicy.java
java -cp "$tmp" TestWatchNextPolicy
cc -D_GNU_SOURCE -std=c11 -Wall -Wextra -Werror -Isrc src/deeplink.c tests/test_watchnext_deeplink.c -pthread -o "$tmp/links"
"$tmp/links"
python3 - <<'CHECK'
from pathlib import Path
import xml.etree.ElementTree as ET
root=ET.parse('android/app/src/main/AndroidManifest.xml').getroot()
ns='{http://schemas.android.com/apk/res/android}'
assert any(e.get(ns+'name')=='com.android.providers.tv.permission.WRITE_EPG_DATA' for e in root.findall('uses-permission'))
assert any(e.get(ns+'name')=='.StreamMaxWatchNextReceiver' for e in root.find('application').findall('receiver'))
app=Path('src/app.c').read_text()
assert 'watchnext_atualizar(agora, tela != TELA_LOGIN && tela != TELA_ESCOLHA_PERFIL)' in app
publisher=Path('android/app/src/main/java/space/nuvio/nativelegacy/StreamMaxWatchNext.kt').read_text()
assert 'cur.getString(1) == c.packageName && key.startsWith(PREFIX)' in publisher
assert 'revision.get() != ticket' in publisher
assert 'WATCH_NEXT_TYPE_CONTINUE' in publisher
assert 'nativeWatchNextDeepLink(request.second, session)' in Path('android/app/src/main/java/space/nuvio/nativelegacy/NuvioActivity.kt').read_text()
print('OK: Android TV publisher ownership, cancellation and native profile gates wired')
CHECK
