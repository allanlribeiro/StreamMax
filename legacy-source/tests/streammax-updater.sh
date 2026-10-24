#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bin="$(mktemp)"
trap 'rm -f "$bin"' EXIT
cc -std=c11 -Wall -Wextra -Werror -I src tests/test_streammax_update.c -o "$bin"
"$bin"
python3 - <<'PY'
from pathlib import Path
import json
native = Path("src/atualizacao.c").read_text()
host = Path("android/app/src/main/java/space/nuvio/nativelegacy/NuvioActivity.kt").read_text()
manifest = Path("android/app/src/main/AndroidManifest.xml").read_text()
paths = Path("android/app/src/main/res/xml/caminhos_atualizacao.xml").read_text()
config = json.loads(Path("deploy/app/appinfo.json").read_text())
assert config["version"] == "2.0.23"
assert '#define SM_UPDATE_VERSION "1.0.16"' in Path("src/streammax_update.h").read_text()
assert '#define AT_URL    SM_UPDATE_API' in native
assert '#define AT_PAGINA SM_UPDATE_PAGE' in native
assert "sm_update_nova_versao(tag)" in native
assert "sm_update_url_apk_valida(tag, dst)" in native
assert 'snprintf(nome, sizeof nome, "StreamMax-%s.apk", ver)' in native
assert "podeAtualizarApk()" in native
assert "rede_baixar_bin(apkUrl, 600, &n)" in native
assert "strcasecmp(hex, hash) != 0" in native
assert "android_instalar_apk(arq)" in native
assert "apkOficialParaAtualizar(arq)" in host
assert "arquivo.packageName != packageName" in host
assert "novoCodigo <= atualCodigo" in host
assert "oficiais.isNotEmpty() && oficiais == recebidos" in host
assert "PackageManager.GET_SIGNING_CERTIFICATES" in host
assert "Intent.ACTION_VIEW" in host
assert "FileProvider.getUriForFile" in host
assert "canRequestPackageInstalls()" in host
assert "android.permission.REQUEST_INSTALL_PACKAGES" in manifest
assert 'android:resource="@xml/caminhos_atualizacao"' in manifest
assert 'path="dados/atualizacao/"' in paths
print("OK: OTA StreamMax 1.0.16 sem Nuvio upstream; APK/signer/versao verificados")
PY
