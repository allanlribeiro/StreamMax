#!/usr/bin/env python3
"""Confere cadastro e arte de perfil no APK compilado, nos dois ABIs."""
import hashlib
from pathlib import Path
import sys
import zipfile

root = Path(__file__).resolve().parents[1]
with zipfile.ZipFile(sys.argv[1]) as apk:
    art = 'marcas/streammax_profiles.png'
    assert hashlib.sha256(apk.read('assets/art/' + art)).digest() == hashlib.sha256((root / 'deploy/app/art' / art).read_bytes()).digest()
    for abi in ['arm64-v8a', 'armeabi-v7a']:
        native = apk.read(f'lib/{abi}/libmain.so')
        for text in ['Crie sua conta', 'Confirmar senha', 'Não tem conta? Criar conta', '/auth/v1/signup', 'streammax_profiles.png']:
            assert text.encode() in native, (abi, text)
print('OK: cadastro compilado nos dois ABIs e nova arte de perfil identica ao fonte')
