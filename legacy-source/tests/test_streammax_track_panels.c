// Exercise the real UI event handlers and desktop playback simulation, without GL/network.
#define main streammax_unused_app_main
#include "../src/main.c"
#undef main
#include "faixas.h"
#include "legendasui.h"
#include "linguas.h"
#include <assert.h>

static void painelTecla(SDL_Keycode k) {
  SDL_Event e; SDL_zero(e);
  e.type = SDL_KEYDOWN; e.key.keysym.sym = k; faixas_evento(&e);
  e.type = SDL_KEYUP; faixas_evento(&e);
}
static void painelAcao(const char *chave) {
  assert(legendasui_teste_focar_chave(chave));
  painelTecla(SDLK_RETURN);
}
static void painelCaptura(const char *nome) {
  static Uint32 agora = 1000;
  for (int i = 0; i < 100; ++i) {
    agora += 16;
    gfx_novo_quadro(); txt_novo_quadro();
    glViewport(0, 0, 1920, 1080);
    glClearColor(.07f, .10f, .15f, 1); glClear(GL_COLOR_BUFFER_BIT);
    faixas_atualizar(1.f / 60.f, agora);
    faixas_desenhar(agora); plrilha_desenhar(agora);
    tex_bombear(6); SDL_Delay(1);
  }
  glFinish();
  SDL_Surface *img = SDL_CreateRGBSurfaceWithFormat(0, 1920, 1080, 32, SDL_PIXELFORMAT_RGBA32);
  assert(img);
  glReadPixels(0, 0, 1920, 1080, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
  unsigned char *linha = malloc((size_t)img->pitch); assert(linha);
  for (int y = 0; y < img->h / 2; ++y) {
    unsigned char *topo = (unsigned char *)img->pixels + y * img->pitch;
    unsigned char *baixo = (unsigned char *)img->pixels + (img->h - 1 - y) * img->pitch;
    memcpy(linha, topo, img->pitch); memcpy(topo, baixo, img->pitch); memcpy(baixo, linha, img->pitch);
  }
  free(linha);
  assert(SDL_SaveBMP(img, nome) == 0); SDL_FreeSurface(img);
}
static void painelCapturas(void) {
  assert(SDL_InitSubSystem(SDL_INIT_VIDEO) == 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_Window *janela = SDL_CreateWindow("StreamMax UI preview", 0, 0, 1920, 1080, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  assert(janela);
  SDL_GLContext contexto = SDL_GL_CreateContext(janela);
  if (!contexto) fprintf(stderr, "Preview EGL context: %s\n", SDL_GetError());
  assert(contexto);
  gfx_tamanho_alvo(1920, 1080); assert(gfx_iniciar());
  IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG); assert(tex_iniciar(64));
  assert(txt_iniciar("deploy/app", 1.f)); gfx_icones_dir("deploy/app/art");
  faixas_abrir_em(0); painelCaptura("build/tests/audio.bmp");
  faixas_abrir_em(1); painelCaptura("build/tests/legendas.bmp");
  painelAcao("filtro"); painelCaptura("build/tests/idiomas.bmp"); painelTecla(SDLK_ESCAPE);
  painelAcao("sincronia"); painelCaptura("build/tests/sincronia.bmp"); painelTecla(SDLK_ESCAPE);
  painelAcao("aparencia"); painelCaptura("build/tests/aparencia.bmp");
  tex_encerrar(); IMG_Quit(); txt_encerrar(); gfx_encerrar(); SDL_GL_DeleteContext(contexto); SDL_DestroyWindow(janela);
}
int main(void) {
  int col, vol, vel, idiomas, alvo;
  assert(SDL_Init(SDL_INIT_TIMER) == 0);
  VideoSimulacao sim; memset(&sim, 0, sizeof sim);
  sim.nAudio = 1; sim.nLeg = 2; sim.legAtual = -1;
  snprintf(sim.audio[0].idioma, sizeof sim.audio[0].idioma, "pt-br");
  snprintf(sim.audio[0].rotulo, sizeof sim.audio[0].rotulo, "Português (Brasil) · AAC 2.0");
  snprintf(sim.leg[0].idioma, sizeof sim.leg[0].idioma, "pt-br"); sim.leg[0].numero = 1;
  snprintf(sim.leg[1].idioma, sizeof sim.leg[1].idioma, "pt-pt"); sim.leg[1].numero = 2;
  video_simular(&sim);
  assert(!strcmp(ling_nome("pt-br"), "Português (Brasil)"));
  assert(!strcmp(ling_nome("pt-pt"), "Português (Portugal)"));

  faixas_abrir_em(0);
  faixas_teste_foco(&col, &vol, &vel); assert(col == 0);
  painelTecla(SDLK_ESCAPE); assert(!faixas_aberta());
  faixas_abrir_em(1);
  faixas_teste_foco(&col, &vol, &vel); assert(col == 1);
  assert(legendasui_teste_pagina(&idiomas) == 0 && !idiomas);

  painelAcao("filtro");
  assert(legendasui_teste_pagina(&idiomas) == 0 && idiomas);
  painelTecla(SDLK_DOWN); assert(video_legenda_atual() == -1); // browsing never selects a track
  painelTecla(SDLK_ESCAPE);
  assert(faixas_aberta());
  assert(legendasui_teste_pagina(&idiomas) == 0 && !idiomas);

  painelAcao("sincronia"); assert(legendasui_teste_pagina(NULL) == 1);
  int antes = player_leg_estilo()->atrasoMs;
  painelTecla(SDLK_RIGHT); assert(player_leg_estilo()->atrasoMs == antes + 100);
  painelTecla(SDLK_LEFT); assert(player_leg_estilo()->atrasoMs == antes);
  painelTecla(SDLK_ESCAPE);
  assert(legendasui_teste_pagina(NULL) == 0);
  assert(!strcmp(legendasui_teste_chave(), "sincronia"));

  painelAcao("opcoes"); assert(legendasui_teste_pagina(NULL) == 2);
  legendasui_teste_foco(&alvo, NULL); assert(alvo == 1);
  painelAcao("sincronia"); assert(legendasui_teste_pagina(NULL) == 1);
  painelTecla(SDLK_ESCAPE); assert(legendasui_teste_pagina(NULL) == 2);
  painelTecla(SDLK_ESCAPE); assert(legendasui_teste_pagina(NULL) == 0);
  assert(!strcmp(legendasui_teste_chave(), "opcoes"));
  legendasui_teste_foco(&alvo, NULL); assert(alvo == 0);

  painelAcao("aparencia");
  faixas_teste_foco(&col, &vol, &vel); assert(col == 2);
  painelTecla(SDLK_ESCAPE);
  faixas_teste_foco(&col, &vol, &vel); assert(col == 1 && faixas_aberta());
  assert(legendasui_teste_focar_chave("filtro"));
  painelTecla(SDLK_RIGHT);
  assert(!strcmp(legendasui_teste_chave(), "sincronia"));
  faixas_teste_foco(&col, &vol, &vel); assert(col == 1); // no accidental jump to style
  painelTecla(SDLK_ESCAPE); assert(!faixas_aberta());
  puts("PASS: separate audio/subtitle panels, language chooser, sync, secondary controls and Back navigation");
  if (getenv("STREAMMAX_PANEL_SCREENSHOTS")) painelCapturas();
  SDL_Quit(); return 0;
}
