#!/usr/bin/env python3
"""Executa foco, geometria e bloco horizontal reais em fixtures sem janela."""
from pathlib import Path
import argparse
import os
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--validate-candidate', action='store_true',
                    help='Antes de editar o app, prova o defeito e testa a correcao em memoria')
parser.add_argument('--source', type=Path, help='Snapshot da Home para reproduzir uma revisao anterior')
args = parser.parse_args()
text = (args.source or ROOT / 'src/home.c').read_text()


def candidato(source):
    old = '    if (r == foco.fileira\n#ifdef NV_TOUCH_UI'
    assert source.count(old) == 1, 'A fonte nao corresponde a baseline esperada'
    source = source.replace(old, '    if ((r == foco.fileira || velX[r] != 0.0f)\n#ifdef NV_TOUCH_UI')
    source = source.replace(
        '      float abre = (r == expFileira && foco.coluna == expColuna) ? expAbre : 0.0f;\n'
        '      float alvo = alvoScrollFil(r, foco.coluna, scrollX[r], abre);',
        '      int col = r == foco.fileira ? foco.coluna : foco.colunaLembrada[r];\n'
        '      float abre = (r == expFileira && col == expColuna) ? expAbre : 0.0f;\n'
        '      float alvo = alvoScrollFil(r, col, scrollX[r], abre);')
    end = '                                       NV_MOLA2_SCROLL, motionReduzido);\n    }'
    assert source.count(end) == 1
    return source.replace(end,
        '                                       NV_MOLA2_SCROLL, motionReduzido);\n'
        '      if (r != foco.fileira && fabsf(velX[r]) <= 0.25f &&\n'
        '          fabsf(scrollX[r] - alvo) <= 0.25f) {\n'
        '        scrollX[r] = alvo; velX[r] = 0.0f;\n'
        '      }\n    }')


def executar(source, baseline=False):
    ini = source.index('static float alvoScrollFil(')
    geometry = source[ini:source.index('\n}', ini)+2]
    ini = source.index('    int nAnim = fileiras[r].n')
    step = re.search(r'    if \(\(?r == foco\.fileira.*?^    }', source[ini:], re.S | re.M)
    assert step, 'Bloco horizontal nao encontrado'
    template = (ROOT / 'tests/test_home_scroll_idle.c.in').read_text()
    code = template.replace('/* GEOMETRIA_REAL */', geometry).replace('/* PASSO_REAL */', step.group())
    with tempfile.TemporaryDirectory(prefix='sm-home-scroll-') as tmp:
        d = Path(tmp); p = d / 'check.c'; p.write_text(code)
        for android, touch in [(False, False), (False, True), (True, False), (True, True)]:
            flags = (['-DNV_ANDROID'] if android else []) + (['-DNV_TOUCH_UI'] if touch else [])
            if baseline:
                flags += ['-DSM_EXPECT_STALE']
            if os.environ.get('SM_SANITIZERS'):
                flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
            binary = d / f'check-{android}-{touch}'
            subprocess.run([os.environ.get('CC', 'cc'), '-std=gnu11', '-O1', '-g',
                            '-I', str(ROOT/'src'), *flags, str(p), str(ROOT/'src/focus.c'),
                            '-lm', '-o', str(binary)], check=True)
            print(f'Android={android} toque={touch} baseline={baseline}', flush=True)
            subprocess.run([str(binary)], check=True)


if args.validate_candidate:
    executar(text, baseline=True)
    executar(candidato(text))
    print('Candidato aprovado sem modificar o codigo do aplicativo')
else:
    executar(text)
