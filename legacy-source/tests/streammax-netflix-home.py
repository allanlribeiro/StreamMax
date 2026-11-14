#!/usr/bin/env python3
"""Actual Netflix hero/CW renderers and shared geometry, with example catalog/text.
These captures are native GLES component fixtures, not the TCL or a live account.
"""
import importlib.util,os,shlex,subprocess,sys,tempfile
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('preview',ROOT/'tests/streammax-hero-preview.py');h=importlib.util.module_from_spec(spec);spec.loader.exec_module(h)
out=Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/streammax-netflix-preview').resolve();out.mkdir(parents=True,exist_ok=True)
cc=shlex.split(os.getenv('CC','cc'));flags=shlex.split(os.getenv('SM_CFLAGS',''));libs=shlex.split(os.getenv('SM_GL_LIBS','-lEGL -lGLESv2 -lSDL2'))
common=['-std=gnu11','-O1','-g','-ffunction-sections','-fdata-sections','-I',str(ROOT/'src'),*flags]
with tempfile.TemporaryDirectory(prefix='sm-netflix-test-') as name:
 d=Path(name);h.fixtures(d)
 play=Image.new("RGBA",(32,32));ImageDraw.Draw(play).polygon([(8,4),(27,16),(8,28)],fill="white");h.raw(d/"play-icon.rgba",play)
 for n in ['02','12']:
  h.raw(d/f'frame-{n}.rgba',Image.open(ROOT/f'deploy/app/art/{n}.jpg'));h.raw(d/f'logo-{n}.rgba',Image.open(ROOT/f'deploy/app/art/logo/{n}.png'))
 im=Image.open(ROOT/'deploy/app/art/marcas/streammax_wordmark.png') if (ROOT/'deploy/app/art/marcas/streammax_wordmark.png').exists() else None
 if im:h.raw(d/'brand.rgba',im)
 labels={'nav':('Início     Explorar     TV ao vivo     Biblioteca     Ajustes',26,'Medium'), 'play':('Assistir',25,'Medium'), 'resume':('Retomar',25,'Medium'), 'info':('Mais informações',25,'Medium'), 'continue':('Continuar assistindo',34,'Bold'), 'row2':('Sua próxima história',34,'Bold'), 'episode':('T3 E1 · O recomeço',25,'Regular'), 'remaining':('15min Restantes',25,'Regular'), 'title':('O Astronauta',28,'Bold'), 'note':('PRÉVIA NATIVA · ARTES E METADADOS DE EXEMPLO',16,'Regular')}
 for n,(text,size,weight) in labels.items():
  font=ImageFont.truetype(str(ROOT/f'deploy/app/fonts/InterDisplay-{weight}.ttf'),size);b=font.getbbox(text);im=Image.new('RGBA',(b[2]+3,b[3]-b[1]+3));ImageDraw.Draw(im).text((0,-b[1]),text,font=font,fill='white');h.raw(d/f'{n}.rgba',im)
  if n in ('play','resume'):
   ink=Image.new('RGBA',im.size,(18,18,18));ink.putalpha(im.getchannel('A'));h.raw(d/f'{n}-ink.rgba',ink)
 template=(ROOT/'tests/test_netflix_renderer.c.in').read_text();old=(ROOT/'tests/test_hero_preview.c.in').read_text();io=old[old.index('static char assetDir'):old.index('int main(')];io=io.replace('assert(f);','if(!f)fprintf(stderr,"Missing fixture: %s\\n",path);assert(f);')
 home=(ROOT/'src/home.c').read_text();cw=(ROOT/'src/continuar.c').read_text();net=(ROOT/'src/home_netflix.inc').read_text()
 template=template.replace('/* REAL_GFX */','#include "'+str(ROOT/'src/gfx.c')+'"').replace('/* FIXTURE_IO */',io).replace('/* REAL_COPY */',h.function(home,'heroCopyLayout')).replace('/* REAL_NETFLIX */',net).replace('/* REAL_CONTINUE */',h.function(cw,'continuarNetflix'))
 (d/'renderer.c').write_text(template)
 subprocess.run(cc+common+['-DNV_ANDROID','-DNV_TOUCH_UI',str(d/'renderer.c'),'-Wl,--gc-sections',*libs,'-lm','-o',str(d/'renderer')],check=True)
 subprocess.run([str(d/'renderer'),str(d)],check=True,env={**os.environ,'EGL_PLATFORM':'surfaceless','MESA_SHADER_CACHE_DISABLE':'true'})
 for n in ['inicio','continuar','catalogo']:
  im=Image.frombytes('RGBA',(1920,1080),(d/f'{n}-overlay.raw').read_bytes()).transpose(Image.Transpose.FLIP_TOP_BOTTOM);im.convert('RGB').save(out/f'{n}.png')
 nav=(ROOT/'tests/test_netflix_geometry.c.in').read_text().replace('/* REAL_SCROLL */','\n'.join(h.function(home,n) for n in ['sobraDireitaFoco','alvoScrollFil'])).replace('/* REAL_NAV */','\n'.join(h.function(net,n) for n in ['netflixClique','netflixTecla']))
 (d/'nav.c').write_text(nav);san=['-fsanitize=address,undefined','-fno-omit-frame-pointer'] if os.getenv('SM_SANITIZERS') else []
 subprocess.run(cc+common+san+[str(d/'nav.c'),str(ROOT/'src/focus.c'),'-Wl,--gc-sections','-lm','-o',str(d/'nav')],check=True);subprocess.run([str(d/'nav')],check=True)
 subprocess.run(cc+common+san+[str(ROOT/'tests/test_globoplay_settings.c'),'-Wl,--gc-sections','-lm','-o',str(d/'settings')],check=True);subprocess.run([str(d/'settings')],check=True)
 subprocess.run(cc+common+['-DNV_ANDROID','-DNV_TOUCH_UI','-fsyntax-only',*[str(ROOT/'src'/f) for f in ['home.c','ajustes.c','menu.c','continuar.c','gfx.c','trailer.c']]],check=True)
print('OK: optional Netflix mode; actual hero/CW GLES, large covers, shared focus/scroll, settings and complete C translation checks')
