#!/usr/bin/env python3
"""Previa tecnica: layout/enquadramento/shader reais, midia e fileiras fixtures.

Nao e captura da TCL nem execucao completa da Home. A composicao CPU dos dois
planos reproduz source-over premultiplicado do Android, sem um decoder real.
"""
import argparse
import json
import os
from pathlib import Path
import re
import shlex
import struct
import subprocess
import tempfile

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
REPO = ROOT.parent


def function(source, name):
    m = re.search(r'^(?:static )?\w+ ' + name + r'\(', source, re.M)
    assert m, name
    clean = re.sub(r'"(?:\\.|[^"\\])*"|//[^\n]*|/\*[\s\S]*?\*/',
                   lambda x: ' '*len(x.group()), source)
    start = clean.index('{', m.start())
    depth = 0
    for i in range(start, len(clean)):
        depth += (clean[i] == '{') - (clean[i] == '}')
        if depth == 0:
            return source[m.start():i+1]
    raise AssertionError(name)


def raw(path, im):
    im = im.convert('RGBA')
    path.write_bytes(struct.pack('<II', *im.size) + im.tobytes())


def fixtures(d):
    art = ROOT/'deploy/app/art'
    for n in ['00','01','02','04','09','10','12','13','15']:
        raw(d/f'poster-{n}.rgba', Image.open(art/'poster'/f'{n}.jpg'))
    raw(d/'frame.rgba', Image.open(art/'10.jpg'))
    raw(d/'logo.rgba', Image.open(art/'logo/10.png'))
    fontdir = ROOT/'deploy/app/fonts'
    labels = {
        'meta': ('Filme • Ficção científica • 2024 • 1h48min',21,'Regular'),
        'nota': ('IMDb  5,7',21,'Medium'),
        'sin1': ('Em uma missão solitária no espaço, um astronauta',22,'Regular'),
        'sin2': ('encontra uma presença misteriosa a bordo de sua nave.',22,'Regular'),
        'row1': ('Últimos Lançamentos - Filmes',34,'Bold'),
        'row2': ('Últimos Lançamentos - Séries',34,'Bold'),
        'ver': ('Ver tudo ›',21,'Regular'),
        'clock': ('19:47',25,'Medium'),
        'cardmeta': ('2024 • 1h48min',21,'Regular'),
    }
    for name,(text,size,weight) in labels.items():
        font = ImageFont.truetype(str(fontdir/f'InterDisplay-{weight}.ttf'), size)
        b = font.getbbox(text)
        im = Image.new('RGBA',(b[2]+3,b[3]-b[1]+3))
        ImageDraw.Draw(im).text((0,-b[1]),text,font=font,fill='white')
        raw(d/f'{name}.rgba',im)


def source_at(ref, path):
    if not ref:
        return (REPO/path).read_text()
    return subprocess.check_output(['git','show',f'{ref}:{path}'],cwd=REPO,text=True)


def render(d, out, name, ref=None):
    home = source_at(ref,'legacy-source/src/home.c')
    helper = source_at(ref,'legacy-source/src/trailerhero.h')
    (d/f'{name}-trailerhero.h').write_text(helper)
    template = (ROOT/'tests/test_hero_preview.c.in').read_text()
    names = ['homeCinematicaTV','homeShelfTop','homeCabecalhoH','empurraHeroModerna',
             'heroTrailerArea','heroTrailerVideoRect','heroTrailerBase','heroBaseCopia','heroCopyLayout']
    code = template.replace('/* REAL_HOME */','\n'.join(function(home,n) for n in names))
    code = code.replace('/* REAL_GFX */','#include "'+str(ROOT/'src/gfx.c')+'"')
    code = code.replace('/* REAL_TRAILER_HELPER */','#include "'+str(d/f'{name}-trailerhero.h')+'"')
    cfile = d/f'{name}.c'
    cfile.write_text(code)
    binary = d/name
    subprocess.run(shlex.split(os.getenv('CC','cc'))+[
        '-std=gnu11','-O1','-ffunction-sections','-fdata-sections','-DNV_ANDROID','-DNV_TOUCH_UI',
        '-I',str(ROOT/'src'),*shlex.split(os.getenv('SM_CFLAGS','')),str(cfile),'-Wl,--gc-sections',
        *shlex.split(os.getenv('SM_GL_LIBS','-lEGL -lGLESv2 -lSDL2')),'-lm','-o',str(binary)],check=True)
    meta=json.loads(subprocess.check_output([str(binary),str(d),name],text=True,
        env={**os.environ,'EGL_PLATFORM':'surfaceless','MESA_SHADER_CACHE_DISABLE':'true'}))
    def read(plane):
        return Image.frombytes('RGBA',(1920,1080),(d/f'{name}-{plane}.raw').read_bytes()).transpose(Image.Transpose.FLIP_TOP_BOTTOM)
    # RGB do canvas ja premultiplicado; o video completa a transparencia.
    overlay,video=read('overlay'),read('video')
    inv=Image.eval(overlay.getchannel('A'),lambda a:255-a)
    from PIL import ImageChops
    final=ImageChops.add(overlay.convert('RGB'),ImageChops.multiply(video.convert('RGB'),Image.merge('RGB',(inv,inv,inv))))
    final.save(out/f'{name}.png')
    return meta


def main():
    p=argparse.ArgumentParser()
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--baseline',default='da499ecbb9693ec46979f2dc5ce5939850181090')
    args=p.parse_args();args.output.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='sm-hero-preview-') as temp:
        d=Path(temp);fixtures(d)
        before=render(d,args.output,'antes',args.baseline)
        after=render(d,args.output,'depois')
    assert abs(after['cards_y']/before['cards_y']-1.15)<.0001
    assert after['video_h']<before['video_h']
    assert after['source_bottom_at_fade']>before['source_bottom_at_fade']+.10
    results={'baseline':args.baseline,'antes':before,'depois':after,
             'limitacao':'GLES real; quadro estatico e fileiras de teste; sem decoder ou TV fisica.'}
    (args.output/'medidas.json').write_text(json.dumps(results,indent=2,ensure_ascii=False)+'\n')
    font=ImageFont.truetype(str(ROOT/'deploy/app/fonts/InterDisplay-Bold.ttf'),30)
    board=Image.new('RGB',(1280,1540),'#101013')
    draw=ImageDraw.Draw(board)
    for index,(name,title) in enumerate([('antes','Antes · 1.0.24'),('depois','Depois · Hero +15%')]):
        y=index*770
        draw.text((24,y+12),title,font=font,fill='white')
        board.paste(Image.open(args.output/f'{name}.png').resize((1280,720)),(0,y+50))
    board.save(args.output/'comparativo.png')
    print(json.dumps(results,ensure_ascii=False,indent=2))


if __name__=='__main__':main()
