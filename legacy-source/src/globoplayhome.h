#ifndef SM_GLOBOPLAY_HOME_H
#define SM_GLOBOPLAY_HOME_H
#include <math.h>

// Geometria independente do catalogo e do decoder; coordenadas da camada Home.
// O layout e opcional. Nenhuma medida dos tres layouts anteriores muda.
typedef struct { float x,y,w,h,shelf,push; } SmGloboHome;
static inline SmGloboHome sm_globo_home(float width,float height) {
  SmGloboHome g;
  g.x=width*.10f; g.w=width*.80f;
  g.y=fminf(110.0f,height*.13f);
  g.h=fminf(g.w/2.30f,height*.62f);
  g.shelf=g.y+g.h+52.0f;
  g.push=fmaxf(0,g.shelf-240.0f);
  return g;
}
static inline void sm_globo_video(float x,float y,float w,float h,int iw,int ih,
                                  float *vx,float *vy,float *vw,float *vh) {
  float aspect=iw>0&&ih>0?(float)iw/ih:16.0f/9.0f;
  *vw=fmaxf(w,h*aspect); *vh=*vw/aspect;
  *vx=x+(w-*vw)*.5f; *vy=y+(h-*vh)*.5f;
}
#endif
