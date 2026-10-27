#!/usr/bin/env python3
"""Exercita geometria real e o shader GLES contra as paradas do NuvioTV.

Referencia: NuvioMedia/NuvioTV 252e9eb0ceb3d31d7bcbe770736f6eb83f283e05,
ModernHomeHero.kt/ModernHomeModels.kt. Sem rede, decoder ou janela Android.
"""
import ast
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def function(source, name):
    match = re.search(r'^(?:static )?(?:void|int|GfxRect) ' + name + r'\(', source, re.M)
    assert match, name
    return source[match.start():source.index('\n}', match.start()) + 2]


home = (ROOT / 'src/home.c').read_text()
trailer = (ROOT / 'src/trailer.c').read_text()
gfx = (ROOT / 'src/gfx.c').read_text()
shader_section = gfx[gfx.index('  // GFX_FURO_HERO —'):gfx.index('\n};', gfx.index('  // GFX_FURO_HERO —'))]
shader = ''.join(ast.literal_eval(line.strip().rstrip(',')) for line in shader_section.splitlines()
                 if line.strip().startswith('"'))
assert 'texture2D' not in shader and 'smoothstep' not in shader
common = r'''
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "layout.h"
#include "trailerhero.h"
typedef struct { float x,y,w,h; } GfxRect;
enum { HOME_LAYOUT_MODERNA, HOME_LAYOUT_PADRAO, HOME_LAYOUT_DINAMICA };
static int lay;
static int layoutHome(void) { return lay; }
static GfxRect padBannerRect(void) { return (GfxRect){0,0,1920,650}; }
static float dinHeroY(void) { return -20; }
static int aberto, dono, noCartao, cheia, quadro;
enum { TRAILER_DONO_NENHUM, TRAILER_DONO_HOME, TRAILER_DONO_DETALHE };
static int video_quadro_visto(void) { return quadro; }
'''
geometry = common + '\n'.join(function(home, n) for n in ['heroTrailerArea', 'heroArtworkRect'])
geometry += function(trailer, 'trailer_mostra_video') + r'''
int main(void) {
  lay=HOME_LAYOUT_MODERNA;
  for(int full=0;full<2;full++) {
    GfxRect art=heroArtworkRect(lay,full), clip=heroTrailerArea(art);
    assert(clip.x==art.x && clip.y==art.y && clip.w==art.w && clip.h==art.h);
    assert(clip.w>=1380 && clip.h>=670);
    const int sizes[][2]={{1920,1080},{1920,804},{1080,1080},{720,1280},{0,0}};
    for(unsigned i=0;i<sizeof sizes/sizeof sizes[0];i++) {
      float x,y,w,h, asp=sizes[i][0]?(float)sizes[i][0]/sizes[i][1]:16.0f/9.0f;
      sm_trailer_hero_video(clip.x,clip.y,clip.w,clip.h,sizes[i][0],sizes[i][1],&x,&y,&w,&h);
      assert(x<clip.x && y<clip.y && x+w>clip.x+clip.w && y+h>clip.y+clip.h);
      assert(fabsf(w/h-asp)<0.001f);
      assert(fabsf(x+w/2-(clip.x+clip.w/2))<0.01f);
      assert(fabsf(y+h/2-(clip.y+clip.h/2))<0.01f);
    }
  }
  lay=HOME_LAYOUT_PADRAO;
  GfxRect pad=heroTrailerArea(padBannerRect()); assert(pad.h==650 && pad.x==0);
  lay=HOME_LAYOUT_DINAMICA;
  GfxRect din=heroTrailerArea(heroArtworkRect(lay,0));
  assert(din.y+din.h<=NV_DIN_REPOUSO_FIL-120.0f);
  dono=TRAILER_DONO_HOME; aberto=1; quadro=0; assert(!trailer_mostra_video());
  quadro=1; assert(trailer_mostra_video());
  aberto=0; assert(!trailer_mostra_video());
  dono=TRAILER_DONO_DETALHE; assert(trailer_mostra_video());
  dono=TRAILER_DONO_HOME; noCartao=1; assert(trailer_mostra_video());
  puts("OK: caixa da arte preservada, cover/overscan uniforme e primeiro quadro real");
}
'''

with tempfile.TemporaryDirectory(prefix='streammax-hero-original-') as tmp:
    d = Path(tmp)
    cc = shlex.split(os.environ.get('CC', 'cc'))
    flags = ['-std=gnu11', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT / 'src')]
    (d / 'geometry.c').write_text(geometry)
    sanitizers = ['-fsanitize=address,undefined', '-fno-omit-frame-pointer'] if os.getenv('SM_SANITIZERS') == '1' else []
    subprocess.run(cc + flags + sanitizers + ['-DNV_ANDROID', str(d / 'geometry.c'), '-lm', '-o', str(d / 'geometry')], check=True)
    subprocess.run([str(d / 'geometry')], check=True)
    (d / 'shader.h').write_text('static const char *fragment_body = ' + __import__('json').dumps(shader) + ';\n')
    template = (ROOT / 'tests/test_hero_original_gles.c.in').read_text()
    template = template.replace('/* PRODUCTION_COMPOSITOR */', function(gfx, 'gfx_furo_hero_suave'))
    (d / 'gles.c').write_text(template)
    inc = shlex.split(os.environ.get('SM_CFLAGS', ''))
    libs = shlex.split(os.environ.get('SM_GL_LIBS', '-lEGL -lGLESv2'))
    subprocess.run(cc + flags + inc + ['-I', str(d), str(d / 'gles.c')] + libs + ['-lm', '-o', str(d / 'gles')], check=True)
    subprocess.run([str(d / 'gles')], check=True, env={**os.environ, 'EGL_PLATFORM': 'surfaceless', 'MESA_SHADER_CACHE_DISABLE': 'true'})
