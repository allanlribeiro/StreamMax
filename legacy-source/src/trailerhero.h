#ifndef SM_TRAILERHERO_H
#define SM_TRAILERHERO_H
#include <math.h>

/* A caixa pertence ao layout; a proporcao da fonte nunca a encolhe.
 * Cover uniforme com margem minima, em vez de zoom fixo de 35%.
 * Fontes ~16:9 podem trazer barras de cinema dentro do quadro: preservar
 * cobertura de um conteudo ate 2.40:1, como o overscan antigo, ampliando
 * apenas quando a proporcao da caixa realmente exige. Nao e deteccao de
 * pixels: fontes com barras atipicas ainda precisam de teste na TV. */
#define SM_TRAILER_OVERSCAN_MIN 1.02f
#define SM_TRAILER_TRANSICAO_S   0.480f

static inline void sm_trailer_hero_video(float x, float y, float w, float h,
                                       int videoW, int videoH,
                                       float *vx, float *vy, float *vw, float *vh) {
  float aspecto = videoW > 0 && videoH > 0 ? (float)videoW / videoH : 16.0f / 9.0f;
  float largura = fmaxf(w, h * aspecto);
  float zoom = SM_TRAILER_OVERSCAN_MIN;
  if (aspecto >= 1.70f && aspecto <= 1.85f)
    zoom = fmaxf(zoom, h * 2.40f / largura * SM_TRAILER_OVERSCAN_MIN);
  *vw = largura * zoom;
  *vh = *vw / aspecto;
  *vx = x + (w - *vw) * 0.5f;
  *vy = y + (h - *vh) * 0.5f;
}
static inline float sm_trailer_hero_fade(float atual, float alvo, float dt,
                                       int reduzido) {
  if (reduzido) return alvo;
  float passo = fmaxf(0.0f, dt) / SM_TRAILER_TRANSICAO_S;
  return alvo > atual ? fminf(alvo, atual + passo) : fmaxf(alvo, atual - passo);
}
#endif
