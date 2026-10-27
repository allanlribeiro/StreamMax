#!/usr/bin/env python3
"""Renderizador GLES completo + geometria Home real + janela usada no Android.

Nao substitui teste fisico: reproduz limites, escala e FIT/COVER em host.
SM_BASELINE_GFX aponta para gfx.c/headers da 1.0.22 para reproduzir fade tardio.
"""
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def function(source, name):
    m = re.search(r'^(?:static )?(?:void|int|float|GfxRect) ' + name + r'\(', source, re.M)
    assert m, name
    # Inclui tambem funcoes curtas fechadas na mesma linha.
    clean = re.sub(r'"(?:\\.|[^"\\])*"|//[^\n]*|/\*[\s\S]*?\*/',
                   lambda x: ' '*len(x.group()), source)
    start = clean.index('{', m.start())
    depth = 0
    for i in range(start, len(clean)):
        depth += (clean[i] == '{') - (clean[i] == '}')
        if depth == 0:
            return source[m.start():i+1]
    raise AssertionError(name)


home = (ROOT/'src/home.c').read_text()
trailer = (ROOT/'src/trailer.c').read_text()
video = (ROOT/'src/video_android.c').read_text()
with tempfile.TemporaryDirectory(prefix='streammax-trailer-edges-') as tmp:
    d = Path(tmp)
    baseline = os.getenv('SM_BASELINE_GFX')
    gfx = Path(baseline).resolve() if baseline else ROOT/'src/gfx.c'
    template = (ROOT/'tests/test_trailer_edges_renderer.c.in').read_text()
    template = template.replace('/* REAL_GFX */', '#include "'+str(gfx)+'"')
    funcs='\n'.join(function(home,n) for n in ['heroTrailerArea','heroTrailerVideoRect','heroTrailerBase'])
    template=template.replace('/* REAL_HOME */', funcs)
    template=template.replace('/* REAL_VIDEO_WINDOW */', '\n'.join(function(video,n) for n in ['video_android_janela_preencher','video_recorte_reaplicar']))
    template=template.replace('/* REAL_TRAILER_WINDOW */',function(trailer,'nativoJanela')+'\n'+function(trailer,'trailer_mostra_video'))
    (d/'test.c').write_text(template)
    cc=shlex.split(os.getenv('CC','cc'))
    inc=shlex.split(os.getenv('SM_CFLAGS',''))
    libs=shlex.split(os.getenv('SM_GL_LIBS','-lEGL -lGLESv2 -lSDL2'))
    flags=['-std=gnu11','-O1','-ffunction-sections','-fdata-sections','-DNV_ANDROID','-DNV_TOUCH_UI',
           '-I',str(ROOT/'src')]
    if baseline: flags+=['-DSM_BAD_BASELINE']
    subprocess.run(cc+flags+inc+[str(d/'test.c'),'-Wl,--gc-sections']+libs+['-lm','-o',str(d/'test')],check=True)
    subprocess.run([str(d/'test')],check=True,env={**os.environ,'EGL_PLATFORM':'surfaceless','MESA_SHADER_CACHE_DISABLE':'true'})
    java=ROOT/'android/app/src/main/java/space/nuvio/nativelegacy/StreamMaxVideoWindow.java'
    subprocess.run(['javac','-d',str(d),str(java),str(ROOT/'tests/TestVideoWindow.java')],check=True)
    subprocess.run(['java','-cp',str(d),'TestVideoWindow'],check=True)
    # Integracao: o modo inteiro chega ao helper, sem virar booleano FIT.
    host=(ROOT/'android/app/src/main/java/space/nuvio/nativelegacy/NvPlayer.kt').read_text()
    native=(ROOT/'src/video_android.c').read_text()
    assert 'definirJanela(x, y, w, h, encaixa, pedido)' in host
    assert 'StreamMaxVideoWindow.calculate(pedX,pedY,pedW,pedH,videoW,videoH,pedEncaixa)' in host
    assert 'C.VIDEO_SCALING_MODE_SCALE_TO_FIT_WITH_CROPPING' in host
    assert 'janelaSeMudou(x,y,w,h,2)' in native
    assert 'janelaKt(ultX, ultY, ultW, ultH, ultModo)' in native
