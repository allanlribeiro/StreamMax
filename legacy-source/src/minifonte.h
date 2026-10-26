// Miniaturas publicadas pela propria fonte. Parser puro: sem rede, SDL ou decoder de video.
#ifndef SM_MINIFONTE_H
#define SM_MINIFONTE_H
#include "seekrvtt.h"
#include <stddef.h>
#define MINI_URL 4096
#define MINI_CUES 16384
#define MINI_FOLHAS 512
#define MINI_PIXELS 6000000u
#define MINI_DOCUMENTO (2u * 1024u * 1024u)
#define MINI_IMAGEM (6u * 1024u * 1024u)
int minifonte_url(const char *base, const char *ref, char *dst, size_t tam);
int minifonte_mesma_origem(const char *a, const char *b);
int minifonte_e_hls(const char *url);
// Extensao declarada pelo addon: thumbnailVtt, ou thumbnails:{url,type:"vtt"}.
// Aceita no proprio stream ou no seu behaviorHints; nunca usa thumbnail/poster estatico.
int minifonte_metadados(const char *obj, const char *fim, char *dst, size_t tam);
int minifonte_vtt(const char *texto, const char *base, SeekrVtt *out);
int minifonte_cue(const SeekrVtt *v, long posMs);
int minifonte_hls_indice(const char *texto, const char *base, char *url, size_t tam, int *w, int *h);
int minifonte_hls_imagens(const char *texto, const char *base, int w, int h, SeekrVtt *out);
// Valida as dimensoes antes de entregar bytes nao confiaveis ao decodificador.
int minifonte_imagem_tamanho(const unsigned char *b, size_t n, int *w, int *h);
#endif
