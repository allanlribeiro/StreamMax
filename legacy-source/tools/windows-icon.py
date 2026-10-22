#!/usr/bin/env python3
"""Build the exact approved StreamMax S into the Windows executable and installer.

The PNG source is the same artwork packaged in the StreamMax Android launcher.
No typographic approximation or substitute glyph is permitted.
"""
from pathlib import Path
from PIL import Image

root = Path(__file__).resolve().parent.parent
source = root / "deploy/app/art/marcas/streammax_icon.png"
output = root / "build/windows/streammax.ico"
if not source.is_file():
    raise SystemExit(f"Logo oficial ausente: {source}")
with Image.open(source) as img:
    img = img.convert("RGBA")
    if min(img.size) < 256 or img.width != img.height:
        raise SystemExit("Logo oficial invalida: necessario PNG quadrado >= 256 px")
    output.parent.mkdir(parents=True, exist_ok=True)
    img.save(output, format="ICO", sizes=[(16, 16), (24, 24), (32, 32),
                                          (48, 48), (64, 64), (128, 128),
                                          (256, 256)])
print(f"Icone do StreamMax Windows: {output}")
