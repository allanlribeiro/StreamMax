#!/usr/bin/env python3
"""Verifica a nova tela nos dois binarios nativos do APK assinado."""
import sys
import zipfile
with zipfile.ZipFile(sys.argv[1]) as apk:
    for abi in ['arm64-v8a', 'armeabi-v7a']:
        native = apk.read(f'lib/{abi}/libmain.so')
        for text in ['Sincronizar pela fala', 'Marcar fala agora', 'Desfazer ajuste pela fala',
                     'Sincronizada pela fala', 'ler trecho completo']:
            assert text.encode() in native, (abi, text)
print('OK: sincronia pela fala, leitura completa e desfazer compilados nos dois ABIs')
