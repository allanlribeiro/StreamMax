#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

python3 - <<'PY'
from pathlib import Path
import xml.etree.ElementTree as ET

native = Path("src/abertura.c").read_text()
brand = Path("src/logoapp.c").read_text()
activity = Path("android/app/src/main/java/space/nuvio/nativelegacy/NuvioActivity.kt").read_text()
drawable = Path("android/app/src/main/res/drawable/abertura.xml")
android_build = Path("tools/android.sh").read_text()
gradle = Path("android/app/build.gradle.kts").read_text()
asset = Path("deploy/app/art/marcas/streammax_icon.png")
android_icon = Path("android/app/src/main/res/drawable-nodpi/streammax_icon.png")

assert asset.is_file() and android_icon.is_file(), "O S oficial nao esta no repositorio"
assert asset.read_bytes() == android_icon.read_bytes(), "Icones Android e SDL divergem"
assert "logoapp_streammax_simbolo(" in native
assert "logoapp_streammax_simbolo(" in brand
assert '"streammax_icon.png"' in brand
assert "abertura_previa(" in native
assert "ABERTURA_TETO_MS" in native
assert "gfx_cor((GfxRect){0, 0, NV_TELA_W, NV_TELA_H}" in native
for unexpected in ("IMG_Load(", "streammax_splash.jpg", "TXT_ILHA_TITULO", "logoapp_streammax_wordmark(", "arteCheia"):
    assert unexpected not in native, f"Abertura nativa contem arte ou texto antigo: {unexpected}"

root = ET.parse(drawable).getroot()
android = "{http://schemas.android.com/apk/res/android}"
assert root.tag == "layer-list"
bitmaps = root.findall("./item/bitmap")
assert len(bitmaps) == 1
assert bitmaps[0].attrib[android + "src"] == "@drawable/streammax_icon"
assert bitmaps[0].attrib[android + "gravity"] == "center"
assert "@drawable/streammax_launch" not in drawable.read_text()

assert "StreamMaxAberturaVideo" not in activity
assert not Path("android/app/src/main/java/space/nuvio/nativelegacy/StreamMaxAberturaVideo.kt").exists()
assert "streammax_abertura.mp4" not in android_build, "MP4 nao pode entrar na APK"
assert "streammax_icon.png" in android_build
assert '"mp4"' not in gradle, "MP4 nao deve ser configurado como vinheta"
print("OK: mesmo S original no Android e no SDL, sem video, texto ou JPEG antigo")
PY
