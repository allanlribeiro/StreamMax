#!/usr/bin/env python3
"""Instala os PNGs da marca aprovada na arvore Legacy. Nao compila APK.

Uso:
 python3 migration/install-brand-assets.py /caminho/streammax_brand_assets

Arquivos exigidos: streammax_icon.png, streammax_banner.png, streammax_splash.png.
"""
from pathlib import Path
import shutil
import struct
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
# Validar TODO o pacote antes de copiar qualquer arquivo.
dimensions = {"icon": (512, 512), "banner": (320, 180), "splash": (1280, 720)}
for name, expected in dimensions.items():
    source = assets / f"streammax_{name}.png"
    if not source.is_file():
        sys.exit(f"PNG oficial ausente: {source}")
    with source.open("rb") as fp:
        header = fp.read(24)
    if len(header) < 24 or header[:8] != bytes((137, 80, 78, 71, 13, 10, 26, 10)):
        sys.exit(f"PNG invalido: {source}")
    width, height = struct.unpack(">II", header[16:24])
    if (width, height) != expected:
        sys.exit(f"Dimensoes incorretas em {source}: {width}x{height}; esperado {expected}")
for name in dimensions:
    shutil.copy2(assets / f"streammax_{name}.png", dest / f"streammax_{name}.png")

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
