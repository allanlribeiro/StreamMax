#!/usr/bin/env python3
"""Testa o JSON real da release com o parser que foi instalado na 1.0.18."""
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
BASE = 'f8ccdb383e5ac5a6b6909d6657a4c75a51d6065f'
EXPECTED_HASH = 'cbdc86b315d2b52af7752af6e0f19dcff483653b292c25c48c0cb9bdc259086c'
EXPECTED_URL = 'https://github.com/allanlribeiro/StreamMax/releases/download/v1.0.19/StreamMax-1.0.19-android.apk'


def git_source(path):
    return subprocess.check_output(['git', 'show', f'{BASE}:{path}'], cwd=ROOT, text=True)


def function(source, name):
    m = re.search(r'^static (?:int|void) ' + name + r'\(', source, re.MULTILINE)
    assert m, name
    return source[m.start():source.index('\n}', m.start()) + 2]


payload = Path(sys.argv[1]).resolve()
release = json.loads(payload.read_text())
assert release['tag_name'] == 'v1.0.19'
assert release['prerelease'] is False
asset = next(a for a in release['assets'] if a['name'] == 'StreamMax-1.0.19-android.apk')
assert asset['state'] == 'uploaded'
assert asset['size'] == 62491978
assert asset['digest'] == 'sha256:' + EXPECTED_HASH
assert asset['browser_download_url'] == EXPECTED_URL
if len(sys.argv) > 2:
    assert hashlib.sha256(Path(sys.argv[2]).read_bytes()).hexdigest() == EXPECTED_HASH

original = git_source('legacy-source/src/atualizacao.c')
header = git_source('legacy-source/src/streammax_update.h')
assert '#define SM_UPDATE_VERSION "1.0.18"' in header
code = '\n'.join([
    '#include <assert.h>\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>',
    '#include "installed-update.h"',
    function(original, 'textoJson'),
    function(original, 'hashAntesDe'),
    function(original, 'acharAnexo'),
    '#define AT_APK_SUFIXO "-android.apk"',
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
  assert(!strcmp(url, "https://github.com/allanlribeiro/StreamMax/releases/download/v1.0.19/StreamMax-1.0.19-android.apk"));
  assert(!strcmp(hash, "cbdc86b315d2b52af7752af6e0f19dcff483653b292c25c48c0cb9bdc259086c"));
  assert(!sm_update_tag_valida("v1.0.19-source-thumbnails-preview.1"));
  free(json);
  puts("OK: atualizador instalado na 1.0.18 reconhece a 1.0.19, URL oficial e SHA-256 do APK");
}
''',
])
with tempfile.TemporaryDirectory(prefix='streammax-ota-') as tmp:
    d = Path(tmp)
    (d / 'installed-update.h').write_text(header)
    (d / 'check.c').write_text(code)
    binary = d / 'check'
    subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-Wall', '-Wextra', '-Werror',
                    '-I', str(d), str(d / 'check.c'), '-o', str(binary)], check=True)
    subprocess.run([str(binary), str(payload)], check=True)
