#!/usr/bin/env python3
"""Valida o APK e o JSON com o atualizador que foi instalado na 1.0.25."""
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
BASE = '6f142be885ccc140bb33e61f5768bf5b759c8d6c'
EXPECTED_URL = 'https://github.com/allanlribeiro/StreamMax/releases/download/v1.0.26/StreamMax-1.0.26-android.apk'
assert len(sys.argv) == 3, 'Informe JSON da release e APK verificado'
payload = Path(sys.argv[1]).resolve()
apk = Path(sys.argv[2]).resolve()
expected_hash = hashlib.sha256(apk.read_bytes()).hexdigest()
release = json.loads(payload.read_text())
assert release['tag_name'] == 'v1.0.26' and release['prerelease'] is False
asset = next(a for a in release['assets'] if a['name'] == apk.name)
assert apk.name == 'StreamMax-1.0.26-android.apk'
assert asset['state'] == 'uploaded' and asset['size'] == apk.stat().st_size
assert asset['digest'] == 'sha256:' + expected_hash
assert asset['browser_download_url'] == EXPECTED_URL


def source(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT, text=True)


def function(text, name):
    m = re.search(r'^static (?:int|void) ' + name + r'\(', text, re.M)
    assert m, name
    return text[m.start():text.index('\n}', m.start()) + 2]


original = source('legacy-source/src/atualizacao.c')
header = source('legacy-source/src/streammax_update.h')
assert '#define SM_UPDATE_VERSION "1.0.25"' in header
code = '\n'.join([
    '#include <assert.h>\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>',
    '#include "installed-update.h"',
    '#define EXPECTED_HASH ' + json.dumps(expected_hash),
    '#define EXPECTED_URL ' + json.dumps(EXPECTED_URL),
    function(original, 'textoJson'), function(original, 'hashAntesDe'),
    function(original, 'acharAnexo'), '#define AT_APK_SUFIXO "-android.apk"',
    function(original, 'acharApk'),
    r'''
int main(int argc, char **argv) {
  assert(argc == 2);
  FILE *f = fopen(argv[1], "rb"); assert(f);
  assert(!fseek(f, 0, SEEK_END)); long size = ftell(f); assert(size > 0);
  rewind(f); char *json = calloc((size_t)size + 1, 1); assert(json);
  assert(fread(json, 1, (size_t)size, f) == (size_t)size); fclose(f);
  char tag[48] = "", url[512] = "", hash[80] = "";
  assert(textoJson(json, "tag_name", tag, sizeof tag));
  assert(sm_update_nova_versao(tag));
  assert(acharApk(json, tag, url, sizeof url, hash, sizeof hash));
  assert(!strcmp(url, EXPECTED_URL) && !strcmp(hash, EXPECTED_HASH));
  free(json);
  puts("OK: atualizador da 1.0.25 reconhece a 1.0.26, URL e SHA-256 corretos");
}
''',
])
with tempfile.TemporaryDirectory(prefix='streammax-126-ota-') as tmp:
    d = Path(tmp)
    (d / 'installed-update.h').write_text(header)
    (d / 'check.c').write_text(code)
    binary = d / 'check'
    subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-Wall', '-Wextra', '-Werror',
                    '-I', str(d), str(d / 'check.c'), '-o', str(binary)], check=True)
    subprocess.run([str(binary), str(payload)], check=True)
