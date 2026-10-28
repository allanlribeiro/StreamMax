#ifndef SM_PROFISSIONAL_HOME_H
#define SM_PROFISSIONAL_HOME_H
#include <math.h>

/* Coordenadas da Home, antes da escala de texto. Sem novas texturas/decoder. */
typedef struct { float margin,heroH,copyW,base,hubY,hubH,shelf,push; } SmProfHome;
static inline SmProfHome sm_prof_home(float w,float h) {
  SmProfHome p;
  p.margin=fminf(96.0f,w*.05f);
  p.heroH=fminf(h*.68f,w*.40f);
  p.copyW=fminf(600.0f,w*.36f);
  p.base=p.heroH-86.0f;
  p.hubY=p.heroH+6.0f;
  p.hubH=100.0f;
  p.shelf=p.hubY+p.hubH+50.0f;
  p.push=fmaxf(0.0f,p.shelf-240.0f);
  return p;
}
/* Cover uniforme; nunca muda a caixa por causa de uma fonte quadrada. */
static inline void sm_prof_video(float x,float y,float w,float h,int iw,int ih,
                                 float *vx,float *vy,float *vw,float *vh) {
  float aspect=iw>0&&ih>0?(float)iw/ih:16.0f/9.0f;
  *vw=fmaxf(w,h*aspect);*vh=*vw/aspect;
  *vx=x+(w-*vw)*.5f;*vy=y+(h-*vh)*.5f;
}
#define SM_PROF_PASTAS_MAX 4
#endif
