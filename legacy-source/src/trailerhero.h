#ifndef SM_TRAILERHERO_H
#define SM_TRAILERHERO_H
#include <math.h>

/* Fit the whole frame at the upper right. The same rectangle is used by
 * the native surface and the alpha mask; never crop or stretch the video. */
static inline void sm_trailer_hero_area(float telaW, float maxW, float maxH,
                                      int videoW, int videoH,
                                      float *x, float *w, float *h) {
  float aspecto = videoW > 0 && videoH > 0 ? (float)videoW / videoH : 16.0f / 9.0f;
  *w = fminf(maxW, maxH * aspecto);
  *h = *w / aspecto;
  *x = telaW - *w;
}
#define SM_TRAILER_FADE_ESQUERDA 0.16f
#define SM_TRAILER_FADE_BASE     0.86f
#endif
