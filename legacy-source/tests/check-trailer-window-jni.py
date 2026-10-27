#!/usr/bin/env python3
"""Confere no APK real a ponte JNI estatica de abertura com janela."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile
import zipfile

p=argparse.ArgumentParser()
p.add_argument('apk',type=Path)
p.add_argument('dexdump',type=Path)
p.add_argument('--method',default='abrirComJanela')
p.add_argument('--signature',default='(Ljava/lang/String;Ljava/lang/String;IIIIIII)V')
a=p.parse_args()
found=False
with zipfile.ZipFile(a.apk) as z,tempfile.TemporaryDirectory(prefix='streammax-jni-') as tmp:
    for name in z.namelist():
        if not re.fullmatch(r'classes\d*\.dex',name):continue
        path=Path(tmp)/name
        path.write_bytes(z.read(name))
        data=subprocess.check_output([str(a.dexdump),'-e',str(path)]).decode('utf-8','replace')
        cls=re.search(r"Class descriptor\s*:\s*'Lspace/nuvio/nativelegacy/NvPlayer;'",data)
        if not cls:continue
        end=data.find('\nClass #',cls.end())
        body=data[cls.end():end if end>=0 else None]
        method=re.search(r"name\s*:\s*'"+re.escape(a.method)+r"'\s*type\s*:\s*'([^']+)'\s*access\s*:\s*0x([0-9a-fA-F]+)",body)
        if method:
            assert method[1]==a.signature and int(method[2],16)&8,'Assinatura JNI deve ser exata e estatica'
            found=True
    assert found,'Metodo JNI nao foi encontrado no APK compilado'
    for abi in ['arm64-v8a','armeabi-v7a']:
        native=z.read('lib/'+abi+'/libmain.so')
        assert a.method.encode()+b'\0' in native and a.signature.encode()+b'\0' in native
print('OK: metodo estatico e descritor JNI conferidos no DEX e nas duas bibliotecas nativas')
