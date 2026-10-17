#!/usr/bin/env python3
"""Confere isolamento dos links Android do StreamMax sem gerar APK."""
from pathlib import Path
import sys
import xml.etree.ElementTree as ET

root = Path(sys.argv[1]) if len(sys.argv) == 2 else Path("legacy-source")
manifest = root / "android/app/src/main/AndroidManifest.xml"
app = root / "android/app/build.gradle.kts"
ns = "{http://schemas.android.com/apk/res/android}"
tree = ET.parse(manifest)
filters = tree.findall(".//intent-filter")
schemes = []
hosts = []
for filt in filters:
    actions = [a.get(ns+"name") for a in filt.findall("action")]
    if "android.intent.action.VIEW" not in actions:
        continue
    for data in filt.findall("data"):
        if data.get(ns+"scheme"): schemes.append(data.get(ns+"scheme"))
        if data.get(ns+"host"): hosts.append(data.get(ns+"host"))
assert schemes.count("streammax") == 1, schemes
assert "nuvio-legacy" not in schemes and "stremio" not in schemes, schemes
assert not any(h in ("imdb.com", "www.imdb.com", "m.imdb.com") for h in hosts), hosts
assert 'applicationId = "com.streammax.tv"' in app.read_text(), "Pacote incorreto"
print("OK: StreamMax possui protocolo próprio e não intercepta URLs de outros apps")
