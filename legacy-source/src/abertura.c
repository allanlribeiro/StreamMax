// Abertura do app (#213). Ver abertura.h.
#include "abertura.h"
#include "gfx.h"
#include "anim.h"
#include "layout.h"
#include "ajustes.h"
#include "logoapp.h"
#include "tex_cache.h"
#include "text.h"
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// StreamMax: usar a mesma base azul-marinho do fundo Android (#080D18).\n// O fundo original da arte antiga pode diferir; o splash StreamMax sera proprio.
// 780x247 a partir de (570,416), o centro em (960,540).
#define AB_FUNDO_R (8.0f / 255.0f)
#define AB_FUNDO_G (13.0f / 255.0f)
#define AB_FUNDO_B (24.0f / 255.0f)
#define AB_MARCA_W 780.0f
// A marca do logo Novo (play + nome empilhados, 1200x895) em 1920x1080.
#define AB_NOVO_W 720.0f
#define AB_CENTRO_Y 540.0f
// Nunca segura a home alem disto, com arte chegando ou nao.
#define ABERTURA_TETO_MS 1500u

// OS TRES ESTILOS (Ajustes > Aparencia > "Abertura do app", 2.0 N1). Cada um e so
// um conjunto de numeros para a MESMA conta de saida, usada pela abertura de
// verdade e pela previa dos Ajustes (abertura_previa):
//   Padrao       parada minima de 350 ms ("le como abriu, nao como piscada"),
//                saida de 560 ms, a marca cresce 1,018 -> 1,09 e esmaece;
//   So esmaece   a mesma parada e a mesma saida, sem crescer (e o caminho das
//                animacoes reduzidas);
//   Direto       sem parada minima e saida curta de 220 ms, sem crescer: assim
//                que a home tem as artes do primeiro quadro, ela aparece.
typedef struct { Uint32 minMs; float saidaMs; int zoom; } AbEstilo;
static AbEstilo abEstilo(int e, int reduzida) {
  AbEstilo r = { 350u, 560.0f, 1 };
  if (e == 2) { r.minMs = 0u; r.saidaMs = 220.0f; r.zoom = 0; }
  else if (e == 1 || reduzida) r.zoom = 0;
  return r;
}
// A saida em `p` (0..1): o veu sai com cubica de saida; a marca, mais depressa.
static void abSaida(float p, float *veu, float *alfaMarca) {
  float q = 1.0f - p, pm = p * 1.6f;
  if (pm > 1.0f) pm = 1.0f;
  *veu = q * q * q;
  *alfaMarca = (1.0f - pm) * (1.0f - pm);
}

// Splash StreamMax: somente o S OFICIAL, obtido da propria arte do icone.
// A textura ja possui tratamento de fundo no logoapp.c. Nenhum video,
// palavra STREAMMAX, JPEG de splash ou textura 16:9 sao decodificados aqui.
#define AB_SIMBOLO_W 432.0f
#define AB_SIMBOLO_H 540.0f
static int fundoFica;
static int estado; // 0 nao iniciou, 1 parado, 2 esmaecendo, 3 acabou
static Uint32 inicio, saidaEm;
static int estiloAb;

void abertura_fundo_fica(int sim) { fundoFica = sim; }

void abertura_iniciar(const char *dirArte) {
  estado = 0;
  fundoFica = 0;
  estiloAb = ajustes_abertura();
  logoapp_iniciar(dirArte); // usa marcas/streammax_icon.png, ja empacotado
}

void abertura_tecla(void) {
  if (estado == 1) { estado = 2; saidaEm = SDL_GetTicks(); }
}

int abertura_ativa(void) { return estado < 3; }

static void acabar(void) {
  estado = 3;
  printf("[abertura] fim em %u ms\\n", (unsigned)(SDL_GetTicks() - inicio));
  fflush(stdout);
}

int abertura_desenhar(Uint32 agora, float dt, int pendentes) {
  float veu = 1.0f, alfaMarca = 1.0f;
  const AbEstilo es = abEstilo(estiloAb, anim_politica_reduzida);
  (void)dt; // O S permanece imovel enquanto o aplicativo inicializa.
  if (estado == 3) return 0;
  if (estado == 0) { estado = 1; inicio = agora; }
  if (estado == 1) {
    Uint32 passou = agora - inicio;
    static long segura = -1;
    if (segura < 0) {
      const char *v = getenv("NUVIO_ABERTURA_MS");
      segura = v ? atol(v) : 0;
    }
    if (segura > 0 && passou < (Uint32)segura) {
      pendentes = 1;
      passou = 0;
    }
    if ((passou >= es.minMs && pendentes <= 0) ||
        passou >= ABERTURA_TETO_MS) {
      estado = 2;
      saidaEm = agora;
    }
  }
  if (estado == 2) {
    const float duracao = (fundoFica && !anim_politica_reduzida)
                             ? 760.0f : es.saidaMs;
    float p = (float)(agora - saidaEm) / duracao;
    if (p >= 1.0f) { acabar(); return 0; }
    abSaida(p, &veu, &alfaMarca);
    if (fundoFica) {
      float q = 1.0f - p;
      veu = q * q * (3.0f - 2.0f * q);
    }
  }
  gfx_cor((GfxRect){0, 0, NV_TELA_W, NV_TELA_H}, 0,
          AB_FUNDO_R, AB_FUNDO_G, AB_FUNDO_B, veu);
  if (alfaMarca > 0.004f) {
    // S original 432x540 no plano virtual 1920x1080, centro exato.
    logoapp_streammax_simbolo(
        (GfxRect){(NV_TELA_W - AB_SIMBOLO_W) * 0.5f,
                  (NV_TELA_H - AB_SIMBOLO_H) * 0.5f,
                  AB_SIMBOLO_W, AB_SIMBOLO_H},
        alfaMarca);
  }
  return 1;
}

// PREVIA DOS AJUSTES: reproduz a mesma abertura so com o S, sem animacao
// de zoom nem palavra; a configuracao de saida ainda controla o fade.
void abertura_previa(GfxRect r, float raio, int estilo, int logo,
                     Uint32 agora, float alfa) {
  const AbEstilo es = abEstilo(estilo, 0);
  const float entra = 200.0f, para = es.minMs ? 900.0f : 120.0f;
  const float ciclo = entra + para + es.saidaMs + 900.0f;
  float t = (float)(agora % (Uint32)ciclo);
  float veu = 1.0f, am = 1.0f;
  float h, w;
  (void)logo; // A marca StreamMax da abertura e fixa, independentemente do tema.
  if (alfa <= 0.004f || r.w < 8.0f || r.h < 8.0f) return;
  if (t < entra) {
    am = veu = t / entra;
  } else if (t < entra + para) {
    // imagem congelada
  } else if (t < entra + para + es.saidaMs) {
    abSaida((t - entra - para) / es.saidaMs, &veu, &am);
  } else {
    am = veu = 0.0f;
  }
  gfx_cor(r, raio, AB_FUNDO_R, AB_FUNDO_G, AB_FUNDO_B, veu * alfa);
  h = r.h * (AB_SIMBOLO_H / NV_TELA_H);
  w = h * (AB_SIMBOLO_W / AB_SIMBOLO_H);
  if (w > r.w * 0.8f) { w = r.w * 0.8f; h = w * (AB_SIMBOLO_H / AB_SIMBOLO_W); }
  if (am * alfa > 0.004f) {
    logoapp_streammax_simbolo(
        (GfxRect){r.x + (r.w - w) * 0.5f,
                  r.y + (r.h - h) * 0.5f, w, h}, am * alfa);
  }
}
