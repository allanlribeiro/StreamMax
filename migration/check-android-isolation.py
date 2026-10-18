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
# Identidade Android visível também deve estar no alias efetivamente habilitado.
app_node = tree.find("./application")
assert app_node is not None
assert app_node.get(ns+"icon") == "@drawable/streammax_icon"
assert app_node.get(ns+"banner") == "@drawable/streammax_banner"
assert app_node.get(ns+"theme") == "@style/Tema.StreamMax"
aliases = tree.findall("./application/activity-alias")
active = [a for a in aliases if a.get(ns+"enabled") == "true"]
assert len(active) == 1, "Launcher deve ter exatamente um alias habilitado"
assert active[0].get(ns+"icon") == "@drawable/streammax_icon"
assert active[0].get(ns+"banner") == "@drawable/streammax_banner"
for resource in ("streammax_icon.xml", "streammax_banner.xml"):
    file = root / "android/app/src/main/res/drawable" / resource
    ET.parse(file)
colors = root / "android/app/src/main/res/values/cores.xml"
color_names = {c.get("name") for c in ET.parse(colors).getroot()}
assert {"streammax_fundo", "streammax_azul", "streammax_ciano", "streammax_violeta"} <= color_names
print("OK: links, launcher e identidade Android StreamMax preservados")
