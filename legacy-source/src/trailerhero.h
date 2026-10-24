#ifndef SM_TRAILERHERO_H
#define SM_TRAILERHERO_H
#include <math.h>

/* Start with the 1.0.15 frame and grow its viewport independently toward
 * the left and bottom. Keep the top-right anchor. Video uses uniform cover
 * below so these different growth factors never stretch the picture. */
#define SM_TRAILER_CRESCE_LARGURA 1.17f
#define SM_TRAILER_CRESCE_ALTURA  1.03f
static inline void sm_trailer_hero_area(float telaW, float maxW, float maxH,
                                      int videoW, int videoH,
                                      float *x, float *w, float *h) {
  float aspecto = videoW > 0 && videoH > 0 ? (float)videoW / videoH : 16.0f / 9.0f;
  *w = fminf(maxW, maxH * aspecto);
  *h = *w / aspecto;
  *w = fminf(telaW, *w * SM_TRAILER_CRESCE_LARGURA);
  *h *= SM_TRAILER_CRESCE_ALTURA;
  *x = telaW - *w;
}

static inline void sm_trailer_hero_video(float x, float y, float w, float h,
                                       int videoW, int videoH,
                                       float *vx, float *vy, float *vw, float *vh) {
  float aspecto = videoW > 0 && videoH > 0 ? (float)videoW / videoH : 16.0f / 9.0f;
  *vw = fmaxf(w, h * aspecto);
  *vh = *vw / aspecto;
  *vx = x + (w - *vw) * 0.5f;
  *vy = y + (h - *vh) * 0.5f;
}
/* 14% relative to the previous fade widths (16% left, 14% bottom),
 * rather than adding 14 percentage points across the picture. */
#define SM_TRAILER_FADE_ESQUERDA (0.16f * 1.14f)
#define SM_TRAILER_FADE_BASE     (1.0f - 0.14f * 1.14f)
#endif
