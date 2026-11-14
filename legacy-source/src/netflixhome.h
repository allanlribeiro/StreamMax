#ifndef SM_NETFLIX_HOME_H
#define SM_NETFLIX_HOME_H
#include <math.h>
/* Physical artwork stays large across UI scales. The same focus/rows/player
 * serve every style; this object only describes Netflix-mode geometry. */
typedef struct {float margin,y,w,h,shelf,push,copyW;} SmNetflixHome;
static inline SmNetflixHome sm_netflix_home(float w,float h){
 SmNetflixHome n;n.margin=fminf(88.0f,w*.05f);n.y=fminf(132.0f,h*.123f);
 n.w=fmaxf(1,w-2*n.margin);n.h=fminf(h*.70f,n.w/2.20f);
 n.shelf=n.y+n.h+32;n.push=fmaxf(0,n.shelf-156);n.copyW=fminf(660,n.w*.39f);return n;
}
#define SM_NETFLIX_POSTER_W 348.0f
#define SM_NETFLIX_POSTER_H 522.0f
#define SM_NETFLIX_CW_CAPTION 90.0f
#endif
