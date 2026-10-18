// Regression: fallback de artes dos cards, inclusive "Continuar assistindo".
// Usa a politica real de artehero.c e simula o sinal de falha do tex_cache.
#include "artehero.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static char erros[10][512];
static int nErros;

static int falhou(const char *u) {
  for (int i = 0; i < nErros; ++i) if (!strcmp(u, erros[i])) return 1;
  return 0;
}
static void falha(const char *u) {
  assert(nErros < 10);
  snprintf(erros[nErros++], sizeof erros[0], "%s", u);
}
static void limpa(void) { nErros = 0; }

int main(void) {
  static CatItem item; // CatItem e grande: manter fora da pilha.
  const char *arte;
  artehero_definir_falhou(falhou);

  snprintf(item.imdb, sizeof item.imdb, "tt22084616:1:5");
  snprintf(item.tipo, sizeof item.tipo, "movie");
  snprintf(item.backdrop, sizeof item.backdrop, "https://art.example/erro.jpg");
  snprintf(item.backdropTmdb, sizeof item.backdropTmdb, "https://tmdb.example/fundo.jpg");
  snprintf(item.poster, sizeof item.poster, "https://art.example/poster.jpg");
  assert(!strcmp(artehero_url_card(&item), item.backdrop));

  falha(item.backdrop);
  assert(!strcmp(artehero_url_card(&item), item.backdropTmdb));
  falha(item.backdropTmdb);
  arte = artehero_url_card(&item);
  assert(arte && !strcmp(arte, "https://images.metahub.space/background/medium/tt22084616/img"));

  falha(arte);
  assert(!strcmp(artehero_url_card(&item), item.poster));
  falha(item.poster);
  arte = artehero_url_card(&item);
  assert(arte && !strcmp(arte, "https://images.metahub.space/poster/medium/tt22084616/img"));
  falha(arte);
  assert(artehero_url_card(&item) == NULL);

  limpa();
  item.backdrop[0] = item.backdropTmdb[0] = item.poster[0] = 0;
  assert(strstr(artehero_url_card(&item), "/tt22084616/img"));

  item.imdb[0] = 0;
  assert(artehero_url_card(&item) == NULL);
  puts("StreamMax: fallback de card/Continuar assistindo OK");
  return 0;
}
