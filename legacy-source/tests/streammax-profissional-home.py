#!/usr/bin/env python3
"""Home Profissional: renderizador real, entradas controladas, sem emulador/TV.

Hero/hubs/fontes usam as funcoes de producao e gfx.c inteiro. Texto, menu,
catalogo e fileiras de exemplo sao fixtures, nao uma captura completa do app.
"""
import importlib.util
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile
from PIL import Image, ImageDraw, ImageFont, ImageChops

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('hero_preview',ROOT/'tests/streammax-hero-preview.py')
helpers=importlib.util.module_from_spec(spec);spec.loader.exec_module(helpers)
out=Path(sys.argv[1] if len(sys.argv)>1 else 'output/profissional-home/preview').resolve()
out.mkdir(parents=True,exist_ok=True)
home=(ROOT/'src/home.c').read_text()
prof=(ROOT/'src/home_profissional.inc').read_text()
flags=shlex.split(os.getenv('SM_CFLAGS',''))
cc=shlex.split(os.getenv('CC','cc'))
common=['-std=gnu11','-O1','-g','-ffunction-sections','-fdata-sections','-I',str(ROOT/'src'),*flags]
san=['-fsanitize=address,undefined','-fno-omit-frame-pointer'] if os.getenv('SM_SANITIZERS') else []

with tempfile.TemporaryDirectory(prefix='sm-profissional-') as temp:
    d=Path(temp);helpers.fixtures(d)
    for n in ['02','12']:
        helpers.raw(d/f'frame-{n}.rgba',Image.open(ROOT/f'deploy/app/art/{n}.jpg'))
    for brand in ['netflix','disney','prime','max']:
        helpers.raw(d/f'brand-{brand}.rgba',Image.open(ROOT/f'deploy/app/art/badges/p-{brand}.webp'))
    helpers.raw(d/'tv-icon.rgba',Image.open(ROOT/'deploy/app/art/icones/aj_tv-minimal-play.png'))
    labels={
        'nav':('Início     Busca     Explorar     Guia TV     Agenda     Biblioteca     Ajustes',25,'Medium','white'),
        'button-dark':('Ver título',25,'Medium','#12171e'),
        'button-light':('Ver título',25,'Medium','white'),
        'tv-title':('TV ao vivo',25,'Medium','white'),
        'tv-sub':('IPTV e VOD',21,'Regular','#afbacb'),
        'sources':('Serviços do catálogo',21,'Regular','#b8c2d0'),
        'generic':('Coleção',25,'Medium','white'),
        'continue':('Continuar assistindo',34,'Bold','white'),
    }
    for name,(text,size,weight,color) in labels.items():
        font=ImageFont.truetype(str(ROOT/f'deploy/app/fonts/InterDisplay-{weight}.ttf'),size)
        b=font.getbbox(text);im=Image.new('RGBA',(b[2]+3,b[3]-b[1]+3))
        ImageDraw.Draw(im).text((0,-b[1]),text,font=font,fill=color);helpers.raw(d/f'{name}.rgba',im)
    old=(ROOT/'tests/test_hero_preview.c.in').read_text()
    io=old[old.index('static char assetDir'):old.index('int main(')]
    code=(ROOT/'tests/test_profissional_renderer.c.in').read_text()
    code=code.replace('/* REAL_GFX */','#include "'+str(ROOT/'src/gfx.c')+'"')
    code=code.replace('/* FIXTURE_IO */',io).replace('/* REAL_COPY */',helpers.function(home,'heroCopyLayout'))
    code=code.replace('/* REAL_PROF */',prof)
    (d/'renderer.c').write_text(code)
    libs=shlex.split(os.getenv('SM_GL_LIBS','-lEGL -lGLESv2 -lSDL2'))
    subprocess.run(cc+common+['-DNV_ANDROID',str(d/'renderer.c'),'-Wl,--gc-sections',*libs,'-lm','-o',str(d/'renderer')],check=True)
    subprocess.run([str(d/'renderer'),str(d)],check=True,env={**os.environ,'EGL_PLATFORM':'surfaceless','MESA_SHADER_CACHE_DISABLE':'true'})
    for name in ['inicio','colecoes','fileiras']:
        def plane(p):
            return Image.frombytes('RGBA',(1920,1080),(d/f'{name}-{p}.raw').read_bytes()).transpose(Image.Transpose.FLIP_TOP_BOTTOM)
        overlay,video=plane('overlay'),plane('video')
        inv=Image.eval(overlay.getchannel('A'),lambda a:255-a)
        rgb=ImageChops.add(overlay.convert('RGB'),ImageChops.multiply(video.convert('RGB'),Image.merge('RGB',(inv,inv,inv))))
        rgb.save(out/f'{name}.png')
    nav_names=['profissionalHubs','profissionalEscolher','profissionalClique','profissionalTecla','profissionalColecoes']
    nav=(ROOT/'tests/test_profissional_nav.c.in').read_text().replace('/* REAL_NAV */','\n'.join(helpers.function(prof,n) for n in nav_names))
    (d/'nav.c').write_text(nav)
    subprocess.run(cc+common+san+[str(d/'nav.c'),str(ROOT/'src/focus.c'),'-Wl,--gc-sections','-lm','-o',str(d/'nav')],check=True)
    subprocess.run([str(d/'nav')],check=True)
    subprocess.run(cc+common+san+[str(ROOT/'tests/test_globoplay_settings.c'),'-Wl,--gc-sections','-lm','-o',str(d/'settings')],check=True)
    subprocess.run([str(d/'settings')],check=True)
    # Traducoes completas, incluindo caminhos de controle remoto/ponteiro.
    subprocess.run(cc+common+['-DNV_ANDROID','-DNV_TOUCH_UI','-fsyntax-only',*[str(ROOT/'src'/f) for f in ['home.c','ajustes.c','gfx.c','trailer.c']]],check=True)
print('OK: Profissional implementado; imagens de demonstracao do Hero/hubs reais, sem medicao de FPS ou teste fisico')
