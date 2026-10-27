#ifndef SM_TRAILERHERO_H
#define SM_TRAILERHERO_H
#include <math.h>

/* NuvioTV ModernHomeModels/ModernHomeHero, commit 252e9eb0:
 * the media box belongs to the layout, never to the video's aspect ratio.
 * Cover and overscan scale both axes equally inside that fixed box. */
#define SM_TRAILER_OVERSCAN_ZOOM 1.35f
#define SM_TRAILER_TRANSICAO_S   0.480f

static inline void sm_trailer_hero_video(float x, float y, float w, float h,
                                       int videoW, int videoH,
                                       float *vx, float *vy, float *vw, float *vh) {
  float aspecto = videoW > 0 && videoH > 0 ? (float)videoW / videoH : 16.0f / 9.0f;
  *vw = fmaxf(w, h * aspecto) * SM_TRAILER_OVERSCAN_ZOOM;
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
