"""Convert the user-approved S mark to a multi-resolution Windows icon."""
from pathlib import Path
from PIL import Image
source = Path(__file__).resolve().parent.parent / "assets" / "streammax-s.png"
target = source.with_name("streammax.ico")
if not source.is_file():
    raise SystemExit("Falta a arte oficial streammax-s.png. Nenhum icone generico sera usado.")
with Image.open(source) as img:
    img = img.convert("RGBA")
    if min(img.size) < 256:
        raise SystemExit("O arquivo da logo deve ter pelo menos 256x256 pixels.")
    if img.width != img.height:
        raise SystemExit("A logo deve ter proporcao 1:1 (quadrada).")
    img.save(target, format="ICO", sizes=[(16,16),(24,24),(32,32),(48,48),(64,64),(128,128),(256,256)])
print(f"Icone de Windows gerado: {target}")
