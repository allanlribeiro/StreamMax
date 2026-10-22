"""Convert the user's approved StreamMax S artwork into Windows ICO.

Prefer the original lossless PNG; a compact JPEG derived from the same approved
artwork may be used in preview builds only when the PNG is not present.
"""
from pathlib import Path
from PIL import Image

assets = Path(__file__).resolve().parent.parent / "assets"
png = assets / "streammax-s.png"
jpg = assets / "streammax-s.jpg"
source = png if png.exists() else jpg
target = assets / "streammax.ico"
if not source.is_file():
    raise SystemExit("Falta a arte S aprovada (streammax-s.png ou .jpg).")
with Image.open(source) as source_image:
    img = source_image.convert("RGBA")
    if min(img.size) < 256:
        raise SystemExit("A logo deve ter pelo menos 256x256 pixels.")
    if img.width != img.height:
        raise SystemExit("A logo deve ter proporcao 1:1 (quadrada).")
    img.save(target, format="ICO", sizes=[(16,16),(24,24),(32,32),(48,48),(64,64),(128,128),(256,256)])
print(f"Icone gerado da arte aprovada: {source.name} -> {target.name}")
