#!/usr/bin/env python3
"""Verify the new layout option is compiled in both native Android libraries."""
from pathlib import Path
import sys,zipfile
assert len(sys.argv)==2,'Informe o APK compilado'
with zipfile.ZipFile(Path(sys.argv[1])) as z:
    for abi in ('arm64-v8a','armeabi-v7a'):
        lib=z.read(f'lib/{abi}/libmain.so')
        assert b'Netflix\0' in lib,(abi,'opcao Netflix ausente')
        assert 'Continuar assistindo expansível'.encode() in lib,(abi,'descricao do modo ausente')
        assert b'Mais informa' in lib,(abi,'acao do hero ausente')
        print(f'OK: {abi}: opcao Netflix e textos do modo presentes')
