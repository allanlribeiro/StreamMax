"""Atualiza apenas a inicialização Samsung, conservando o WASM compilado."""
import hashlib, shutil, subprocess, sys, tempfile, zipfile
from pathlib import Path
package=Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory() as tmp:
 root=Path(tmp)
 with zipfile.ZipFile(package) as z:
  if any('signature' in name.lower() for name in z.namelist()):raise SystemExit('Esperado WGT sem assinatura')
  for member in z.infolist():
   if not (root/member.filename).resolve().is_relative_to(root):raise SystemExit('Caminho inválido')
  z.extractall(root)
 wasm=(root/'index.wasm').read_bytes()
 for name in ['streammax-input.js','streammax-player.js']:
  shutil.copyfile(Path('legacy-source/tools')/name,root/name)
 subprocess.run([sys.executable,'legacy-source/tools/streammax-tizen-webapi.py',str(root/'index.html')],check=True)
 with zipfile.ZipFile(package,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
  for path in sorted(root.rglob('*')):
   if path.is_file():z.write(path,path.relative_to(root).as_posix())
 with zipfile.ZipFile(package) as z:assert z.read('index.wasm')==wasm
 print('WASM preservado, SHA-256:',hashlib.sha256(wasm).hexdigest())
