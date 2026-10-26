#ifndef SM_MINIFONTE_REDE_H
#define SM_MINIFONTE_REDE_H
#include "minifonte.h"
#include "rede.h"
// 1 pronto; 0 sem metadados compativeis; -1 falha de transporte/limite. Nunca consulta Seekr.
int minifonte_carregar(const char *video,const char *vtt,const char *cab,RedeJob *job,SeekrVtt *out);
char *minifonte_baixar(const char *url,const char *video,const char *cab,RedeJob *job,long *n,int *http);
#endif
