#!/usr/bin/env python3
"""Renderizador novo real, dados controlados; nao e emulador nem captura da TV."""
import importlib.util
import os
import re
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile
from PIL import Image,ImageDraw,ImageFont

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('hero_preview',ROOT/'tests/streammax-hero-preview.py')
helpers=importlib.util.module_from_spec(spec);spec.loader.exec_module(helpers)
out=Path(sys.argv[1] if len(sys.argv)>1 else 'output/globoplay-home/preview').resolve()
out.mkdir(parents=True,exist_ok=True)
home=(ROOT/'src/home.c').read_text()
names=['heroCopyLayout','heroNLista','heroPosDe','heroIdxEm','heroIntencao','heroPasso',
       'homeScrollYPasso','globoVizinho','globoPainel','desenhaHeroGloboplay']
# O bloco real de D-pad do Hero, que continua comum aos quatro layouts.
a=home.index('// O DESTAQUE E UM DEGRAU ACIMA DA FILEIRA')
a=home.index('  if (focoHero) {',a);start=home.index('{',a);depth=1;i=start+1
while depth:
    depth+=(home[i]=='{')-(home[i]=='}');i+=1
keyboard=home[a:i]
with tempfile.TemporaryDirectory(prefix='sm-globoplay-') as temp:
    d=Path(temp);helpers.fixtures(d)
    for name,text,size,color in [
        ('nav','Início     TV ao vivo     Filmes     Séries     Pesquisar     Configurações',25,'white'),
        ('button','Ver título',25,'#101010')]:
        font=ImageFont.truetype(str(ROOT/'deploy/app/fonts/InterDisplay-Medium.ttf'),size)
        b=font.getbbox(text);im=Image.new('RGBA',(b[2]+3,b[3]-b[1]+3))
        ImageDraw.Draw(im).text((0,-b[1]),text,font=font,fill=color)
        helpers.raw(d/f'{name}.rgba',im)
    for n in ['02','10','12']:
        helpers.raw(d/f'frame-{n}.rgba',Image.open(ROOT/f'deploy/app/art/{n}.jpg'))
        helpers.raw(d/f'logo-{n}.rgba',Image.open(ROOT/f'deploy/app/art/logo/{n}.png'))
    template=(ROOT/'tests/test_globoplay_renderer.c.in').read_text()
    # Reutiliza somente I/O de texturas e readback do gerador anterior.
    old=(ROOT/'tests/test_hero_preview.c.in').read_text()
    io=old[old.index('static char assetDir'):old.index('int main(')]
    template=template.replace('/* REAL_GFX */','#include "'+str(ROOT/'src/gfx.c')+'"')
    template=template.replace('/* FIXTURE_IO */',io)
    template=template.replace('/* REAL_COPY */',helpers.function(home,'heroCopyLayout'))
    template=template.replace('/* REAL_HOME */','\n'.join(helpers.function(home,n) for n in names if n!='heroCopyLayout'))
    template=template.replace('/* REAL_KEYBOARD */',keyboard)
    template=template.replace('/* REAL_BORDER_CONSTANT */',re.search(r'^#define NV_BORDA_AMP[^\n]+',home,re.M)[0])
    c=d/'test.c';c.write_text(template)
    flags=shlex.split(os.getenv('SM_CFLAGS',''))
    libs=shlex.split(os.getenv('SM_GL_LIBS','-lEGL -lGLESv2 -lSDL2'))
    subprocess.run(shlex.split(os.getenv('CC','cc'))+['-std=gnu11','-O1','-g',
        '-ffunction-sections','-fdata-sections','-DNV_ANDROID','-DNV_TOUCH_UI','-DSDL_GetTicks=sm_ticks',
        '-I',str(ROOT/'src'),*flags,str(c),str(ROOT/'src/focus.c'),'-Wl,--gc-sections',*libs,'-lm','-o',str(d/'test')],check=True)
    subprocess.run([str(d/'test'),str(d)],check=True,
        env={**os.environ,'EGL_PLATFORM':'surfaceless','MESA_SHADER_CACHE_DISABLE':'true'})
    from PIL import ImageChops
    for name in ['inicio','fileiras','transicao']:
        def plane(p):return Image.frombytes('RGBA',(1920,1080),(d/f'{name}-{p}.raw').read_bytes()).transpose(Image.Transpose.FLIP_TOP_BOTTOM)
        overlay,video=plane('overlay'),plane('video')
        inv=Image.eval(overlay.getchannel('A'),lambda a:255-a)
        rgb=ImageChops.add(overlay.convert('RGB'),ImageChops.multiply(video.convert('RGB'),Image.merge('RGB',(inv,inv,inv))))
        rgb.save(out/f'{name}.png')
    # Preferencias reais (compila ajustes.c inteiro com GC de secoes).
    san=['-fsanitize=address,undefined','-fno-omit-frame-pointer'] if os.getenv('SM_SANITIZERS') else []
    subprocess.run(shlex.split(os.getenv('CC','cc'))+['-std=gnu11','-O1','-g',
        '-ffunction-sections','-fdata-sections','-I',str(ROOT/'src'),*flags,*san,
        str(ROOT/'tests/test_globoplay_settings.c'),'-Wl,--gc-sections','-lm','-o',str(d/'settings')],check=True)
    subprocess.run([str(d/'settings')],check=True)
print('OK: imagens usam o novo renderizador real; textos/catalogo/backend de entrada controlados')
