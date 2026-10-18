#!/usr/bin/env python3
"""Instala os PNGs da marca aprovada na arvore Legacy. Nao compila APK.

Uso:
 python3 migration/install-brand-assets.py /caminho/streammax_brand_assets

Arquivos exigidos: streammax_icon.png, streammax_banner.png, streammax_splash.png.
"""
from pathlib import Path
import shutil
import sys
import xml.etree.ElementTree as ET

assets = Path(sys.argv[1]).resolve() if len(sys.argv) == 2 else None
if not assets or not assets.is_dir():
    sys.exit("Informe o diretorio com os tres PNGs oficiais.")
root = Path(__file__).resolve().parent.parent / "legacy-source"
if not (root / "android/app/src/main/AndroidManifest.xml").exists():
    sys.exit("Base Legacy nao importada.")

dest = root / "android/app/src/main/res/drawable-nodpi"
dest.mkdir(parents=True, exist_ok=True)
for name in ("icon", "banner", "splash"):
    source = assets / f"streammax_{name}.png"
    if not source.is_file() or source.read_bytes()[:8] != b"\x89PNG\r\n\x1a\n":
        sys.exit(f"PNG oficial ausente ou invalido: {source}")
    shutil.copy2(source, dest / f"streammax_{name}.png")

manifest = root / "android/app/src/main/AndroidManifest.xml"
xml = manifest.read_text(encoding="utf-8")
xml = xml.replace("@drawable/streammax_icon", "@drawable/streammax_icon")
xml = xml.replace("@drawable/streammax_banner", "@drawable/streammax_banner")
# Os PNGs em drawable-nodpi tem precedencia sobre os vetores homonimos:
# remover os prototipos vetoriais para evitar conflito de recursos no Android.
for name in ("streammax_icon.xml", "streammax_banner.xml"):
    prototype = root / "android/app/src/main/res/drawable" / name
    if prototype.exists():
        prototype.unlink()
manifest.write_text(xml, encoding="utf-8")
# O splash do motor nativo usa uma imagem separada do launcher.
mark = root / "deploy/app/art/marcas"
mark.mkdir(parents=True, exist_ok=True)
shutil.copy2(assets / "streammax_splash.png", mark / "streammax_splash.png")
print("OK: icone, banner e marca do splash copiados. Ainda falta validar no APK.")
