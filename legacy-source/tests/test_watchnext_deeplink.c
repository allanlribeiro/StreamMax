#include "deeplink.h"
#include <assert.h>
#include <stdio.h>
static uint64_t current = 10;
int perfis_sessao_atual(uint64_t generation) { return generation == current; }
int main(void) {
  DeepLink d;
  deeplink_postar_sessao("streammax://play/series/tt0903747/2/3", current);
  assert(deeplink_pegar(&d) && d.sessaoEsperada == 10 && d.temporada == 2 && d.episodio == 3);
  assert(!deeplink_pegar(&d));
  deeplink_postar_sessao("streammax://play/movie/tt4154796", current);
  current++; // Profile changed while login/PIN UI delayed delivery.
  assert(!deeplink_pegar(&d) && d.tipo == DL_NENHUM);
  deeplink_postar_sessao("streammax://play/movie/tt4154796", 10);
  assert(!deeplink_pegar(&d));
  deeplink_postar("streammax://meta/movie/tt4154796");
  assert(deeplink_pegar(&d) && d.tipo == DL_TITULO && !d.sessaoEsperada);
  deeplink_postar_sessao("https://evil.invalid/", current);
  assert(!deeplink_pegar(&d));
  puts("OK: Watch Next links cannot cross account/profile sessions; regular links unchanged");
}
