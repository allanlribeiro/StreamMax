#!/usr/bin/env python3
"""Confirma que o APK contem a opcao e o shader nos dois ABIs publicados."""
from pathlib import Path
import sys
import zipfile

assert len(sys.argv)==2,'Informe o APK compilado'
apk=Path(sys.argv[1])
with zipfile.ZipFile(apk) as z:
    for abi in ['arm64-v8a','armeabi-v7a']:
        info=z.getinfo(f'lib/{abi}/libmain.so')
        assert 0<info.file_size<100_000_000
        lib=z.read(info)
        assert b'Profissional\0' in lib,(abi,'opcao ausente')
        assert b'profEase' in lib,(abi,'shader ausente')
        print(f'OK: {abi} inclui opcao Profissional e shader integrado')
