#ifndef SM_TRAILERHLS_H
#define SM_TRAILERHLS_H
#include <stdio.h>
/* A single bounded video variant with its best Portuguese audio rendition
 * and Portuguese subtitle fallback. No extra network requests or ABR. */
int sm_trailer_hls_reduzir(FILE *master, FILE *saida, int teto, int *largura);
#endif
