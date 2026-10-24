#!/usr/bin/env python3
"""Instala pacotes oficiais, conferindo SHA-256 antes de extrair.
Não executa scripts dos arquivos e não altera o SDK do usuário.
"""
import hashlib, json, os, re, stat, sys, urllib.request, zipfile
from pathlib import Path
root = Path(sys.argv[1]).resolve()
root.mkdir(parents=True, exist_ok=True)
repos = ['https://download.tizen.org/sdk/tizenstudio/official', 'https://download.tizen.org/sdk/extensions/tv_extensions']
packages = {}
for repo in repos:
    catalog = urllib.request.urlopen(repo+'/pkg_list_ubuntu-64', timeout=90).read().decode()
    for block in re.split(r'\n\s*\n(?=Package)',catalog.strip()):
        f = dict(re.findall(r'^([\w-]+) : (.*)$',block,re.M))
        if f.get('Package'): packages[f['Package']] = (repo,f)
done=set()
proof=[]
def install(name):
    if name in done: return
    if name not in packages: raise RuntimeError('Pacote não encontrado: '+name)
    done.add(name)
    repo,p = packages[name]
    for dep in p.get('Install-dependency','').split(','):
        dep=dep.strip().split(' ')[0]
        if dep: install(dep)
    filename = root/(name+'.zip')
    print('Baixando',name,p.get('Version'),flush=True)
    urllib.request.urlretrieve(repo+p['Path'],filename)
    digest=hashlib.file_digest(open(filename,'rb'),'sha256').hexdigest()
    if digest != p['SHA256']: raise RuntimeError('SHA-256 incorreto: '+name)
    with zipfile.ZipFile(filename) as z:
        for member in z.infolist():
            if not member.filename.startswith('data/'): continue
            dest=root/member.filename[5:]
            if not dest.resolve().is_relative_to(root): raise RuntimeError('Caminho inválido')
            if member.is_dir(): dest.mkdir(parents=True,exist_ok=True);continue
            dest.parent.mkdir(parents=True,exist_ok=True)
            metadata=member.external_attr>>16
            if stat.S_ISLNK(metadata):
                link=z.read(member).decode('utf-8')
                if not (dest.parent/link).resolve().is_relative_to(root): raise RuntimeError('Link fora do SDK')
                if dest.exists() or dest.is_symlink(): dest.unlink()
                dest.symlink_to(link)
            else:
                if dest.is_symlink(): dest.unlink()
                dest.write_bytes(z.read(member))
                mode=metadata&0o777
                if mode: dest.chmod(mode)
    filename.unlink()
    proof.append({'package':name,'version':p['Version'],'sha256':digest,'source':repo+p['Path']})
for name in ['sdb','emulator-manager','10.0-emulator-qemu-x86','10.0-emulator-qemu-common','tv-samsung-emulator-qemu-x86','tv-samsung-emulator-qemu-common','tv-samsung-emulator-lib','tv-samsung-emulator-still-image','tv-samsung-emulator-control-panel-devices','TV-SAMSUNG-Public-Emulator']:
    install(name)
(root/'PACKAGES.json').write_text(json.dumps(proof,indent=2))
