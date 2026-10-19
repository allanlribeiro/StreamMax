#include "../src/streammax_flags.h"
#include <assert.h>
#include <string.h>
int main(void) {
  assert(!strcmp(streammax_flag_icone("pt-BR"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("pt_BR"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("Brazilian Portuguese"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("pt-PT"), "sm_flag_pt"));
  assert(!strcmp(streammax_flag_icone("en-US"), "sm_flag_us"));
  assert(!strcmp(streammax_flag_icone("en-GB"), "sm_flag_gb"));
  assert(!strcmp(streammax_flag_icone("es-ES"), "sm_flag_es"));
  assert(!strcmp(streammax_flag_icone("es-MX"), "sm_flag_mx"));
  assert(streammax_flag_icone("pt") == NULL);
  assert(!strcmp(streammax_flag_icone("en"), "sm_flag_us"));
  assert(!strcmp(streammax_flag_icone("EN"), "sm_flag_us"));
  assert(!strcmp(streammax_flag_icone("eng"), "sm_flag_us"));
  assert(streammax_flag_icone("es") == NULL);
  assert(!strcmp(streammax_flag_icone("English"), "sm_flag_us"));
  assert(streammax_flag_icone("Portuguese") == NULL);
  assert(streammax_flag_icone("Dual Audio") == NULL);
  assert(streammax_flag_idioma_legivel("pt-br"));
  assert(!streammax_flag_idioma_legivel(" / / "));
  assert(!streammax_flag_idioma_legivel(" - "));
  assert(!streammax_flag_idioma_legivel(""));
  return 0;
}
