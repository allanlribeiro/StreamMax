// Logo do app. Ver logoapp.h.
#include "logoapp.h"
#include "ajustes.h"
#include "iconeapp.h"
#include "tex_cache.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

static char dirArteLg[512] = ".";
static char caminhos[LOGO_N][3][600];

void logoapp_iniciar(const char *dirArte) {
  int i, f;
  snprintf(dirArteLg, sizeof dirArteLg, "%s", dirArte && dirArte[0] ? dirArte : ".");
  for (i = 0; i < LOGO_N; i++) for (f = 0; f < 3; f++) caminhos[i][f][0] = 0;
}

int logoapp_atual(void) {
  int v = ajustes_logo_app();
  return v == LOGO_CLASSICO ? LOGO_CLASSICO : LOGO_NOVO;
}

const char *logoapp_nome(int logo) {
  return logo == LOGO_NOVO ? "Novo" : logo == LOGO_CLASSICO ? "Clássico" : "";
}

const char *logoapp_caminho(int logo, int forma) {
  static const char *ARQ[LOGO_N][3] = {
    { "logo-novo-simbolo.png", "logo-novo-marca.png", "logo-novo-horizontal.png" },
    { "logo-classico.png",     "abertura.jpg",        "nuvio_wordmark.png" },
  };
  if (logo < 0 || logo >= LOGO_N || forma < 0 || forma > 2) return "";
  if (!caminhos[logo][forma][0])
    snprintf(caminhos[logo][forma], sizeof caminhos[logo][forma], "%s/marcas/%s", dirArteLg, ARQ[logo][forma]);
  return caminhos[logo][forma];
}

int logoapp_miniatura(int logo, GfxRect r, int ladrilho, float alpha) {
  GLuint t;
  GfxRect s = r;
  float asp;
  const char *c;
  if (alpha <= 0.004f) return 0;
  if (ladrilho) gfx_cor(r, 0.22f, 13.0f / 255.0f, 16.0f / 255.0f, 30.0f / 255.0f, alpha);
  c = logoapp_caminho(logo, LOGO_F_SIMBOLO);
  t = tex_obter_larg(c, r.w);
  if (!t) return 0;
  // O simbolo nao e quadrado (o novo e mais alto, o classico mais largo): cabe no
  // ladrilho com ~14% de respiro, centrado.
  asp = tex_aspecto(c);
  if (asp <= 0.0f) asp = 1.0f;
  { float lado = r.w * 0.86f, w = lado, h = lado;
    if (asp >= 1.0f) h = w / asp; else w = h * asp;
    s = (GfxRect){ r.x + (r.w - w) * 0.5f, r.y + (r.h - h) * 0.5f, w, h }; }
  gfx_tex_aspect_atual = 0.0f;
  gfx_rect(s, t, GFX_TEXTO, 0, 0, 0, 0.0f, 1, 1, 1, alpha);
  return 1;
}

int logoapp_marca(GfxRect r, float alpha) {
  GLuint t;
  const char *c;
  float asp, w, h;
  if (iconeapp_marca(r, alpha)) return 1;       // icone de apoiador em vigor vence
  if (logoapp_atual() != LOGO_NOVO || alpha <= 0.004f) return 0;
  c = logoapp_caminho(LOGO_NOVO, LOGO_F_SIMBOLO);
  t = tex_obter_larg(c, r.w);
  if (!t) return 0;
  asp = tex_aspecto(c);
  if (asp <= 0.0f) asp = 0.91f;
  h = r.h; w = h * asp;
  if (w > r.w) { w = r.w; h = w / asp; }
  gfx_tex_aspect_atual = 0.0f;
  gfx_rect((GfxRect){ r.x + (r.w - w) * 0.5f, r.y + (r.h - h) * 0.5f, w, h },
           t, GFX_TEXTO, 0, 0, 0, 0.0f, 1, 1, 1, alpha);
  return 1;
}

// Identidade StreamMax aprovada: usar os pixels das artes oficiais do APK,
// NAO reconstruir a tipografia com TXT_G30B. O PNG original inclui fundo
// escuro; recortar e retirar somente a cor de fundo UMA vez no carregamento.
// Duas texturas pequenas permanecem em memoria, sem trabalho por quadro.
// 0 = simbolo S; 1 = escrita STREAMMAX (sem simbolo).
static GLuint smMarcaTex[2];
static float smMarcaAsp[2];
static int smMarcaTentou[2];

static void smMarcaCarregar(int tipo) {
  static const char *ARQ[] = { "streammax_icon.png", "streammax_banner.png" };
  char caminho[640];
  SDL_Surface *origem = NULL, *rgba = NULL, *corte = NULL;
  SDL_Rect area;
  int y, x;
  if (tipo < 0 || tipo > 1 || smMarcaTentou[tipo]) return;
  smMarcaTentou[tipo] = 1;
  snprintf(caminho, sizeof caminho, "%s/marcas/%s", dirArteLg, ARQ[tipo]);
  origem = IMG_Load(caminho);
  if (!origem) return;
  rgba = SDL_ConvertSurfaceFormat(origem, SDL_PIXELFORMAT_ABGR8888, 0);
  SDL_FreeSurface(origem);
  if (!rgba) return;

  // Regioes medidas nas artes oficiais ja presentes na build RC1.
  // Coordenadas relativas: mantem o recorte caso a resolucao do PNG mude.
  if (tipo == 0) {
    area = (SDL_Rect){rgba->w * 86 / 512, rgba->h * 42 / 512,
                      rgba->w * 342 / 512, rgba->h * 428 / 512};
  } else {
    area = (SDL_Rect){rgba->w * 34 / 512, rgba->h * 193 / 288,
                      rgba->w * 446 / 512, rgba->h * 67 / 288};
  }
  if (area.w <= 0 || area.h <= 0 || area.x + area.w > rgba->w ||
      area.y + area.h > rgba->h) { SDL_FreeSurface(rgba); return; }
  corte = SDL_CreateRGBSurfaceWithFormat(0, area.w, area.h, 32,
                                         SDL_PIXELFORMAT_ABGR8888);
  if (!corte) { SDL_FreeSurface(rgba); return; }
  SDL_SetSurfaceBlendMode(rgba, SDL_BLENDMODE_NONE);
  if (SDL_BlitSurface(rgba, &area, corte, NULL) != 0) {
    SDL_FreeSurface(corte); SDL_FreeSurface(rgba); return;
  }
  SDL_FreeSurface(rgba);

  // Fundo do original: preto-azulado (quase zero). Preserva RGB da marca
  // e suaviza a borda antialias, sem quadrado aparente sobre o fundo da UI.
  for (y = 0; y < corte->h; y++) {
    Uint8 *row = (Uint8 *)corte->pixels + y * corte->pitch;
    for (x = 0; x < corte->w; x++) {
      Uint8 *p = row + x * 4;
      int brilho = p[0] > p[1] ? p[0] : p[1];
      if (p[2] > brilho) brilho = p[2];
      if (brilho <= 28) p[3] = 0;
      else if (brilho < 68) p[3] = (Uint8)((unsigned)p[3] * (unsigned)(brilho - 28) / 40u);
    }
  }
  glGenTextures(1, &smMarcaTex[tipo]);
  if (!smMarcaTex[tipo]) { SDL_FreeSurface(corte); return; }
  glBindTexture(GL_TEXTURE_2D, smMarcaTex[tipo]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, corte->w, corte->h, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, corte->pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  smMarcaAsp[tipo] = (float)corte->w / (float)corte->h;
  gfx_tex_esquecer(0);
  SDL_FreeSurface(corte);
}

static int smMarcaDesenhar(int tipo, GfxRect r, float a) {
  float w, h;
  if (a <= 0.004f || r.w <= 0 || r.h <= 0) return 0;
  smMarcaCarregar(tipo);
  if (!smMarcaTex[tipo] || smMarcaAsp[tipo] <= 0) return 0;
  w = r.w; h = w / smMarcaAsp[tipo];
  if (h > r.h) { h = r.h; w = h * smMarcaAsp[tipo]; }
  gfx_tex_aspect_atual = 0.0f;
  gfx_rect((GfxRect){r.x, r.y + (r.h - h) * 0.5f, w, h},
           smMarcaTex[tipo], GFX_TEXTO, 0, 0, 0, 0.0f,
           1, 1, 1, a);
  return 1;
}

int logoapp_streammax_simbolo(GfxRect r, float alpha) {
  return smMarcaDesenhar(0, r, alpha);
}
int logoapp_streammax_wordmark(GfxRect r, float alpha) {
  return smMarcaDesenhar(1, r, alpha);
}
