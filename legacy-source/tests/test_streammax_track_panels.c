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
  painelTecla(SDLK_RIGHT);
  faixas_teste_foco(&col, &vol, &vel); assert(col == 1); // no accidental jump to style
  painelTecla(SDLK_ESCAPE); assert(!faixas_aberta());
  puts("PASS: separate audio/subtitle panels, language chooser, sync, secondary controls and Back navigation");
  SDL_Quit(); return 0;
}
