#include "../src/streammax_flags.h"
#include <assert.h>
#include <string.h>
int main(void) {
  assert(!strcmp(streammax_flag_icone("pt-BR"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("pt_BR"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("Brazilian Portuguese"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("pt-PT"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("en-US"), "sm_flag_us"));
  assert(!strcmp(streammax_flag_icone("en-GB"), "sm_flag_us"));
  assert(!strcmp(streammax_flag_icone("es-ES"), "sm_flag_es"));
  assert(!strcmp(streammax_flag_icone("es-MX"), "sm_flag_mx"));
  assert(!strcmp(streammax_flag_icone("pt"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("por"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("pt-PT"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("Português (Portugal)"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("en"), "sm_flag_us"));
  assert(!strcmp(streammax_flag_icone("EN"), "sm_flag_us"));
  assert(!strcmp(streammax_flag_icone("eng"), "sm_flag_us"));
  assert(streammax_flag_icone("es") == NULL);
  assert(!strcmp(streammax_flag_icone("English"), "sm_flag_us"));
  assert(!strcmp(streammax_flag_icone("Portuguese"), "sm_flag_br"));
  assert(!strcmp(streammax_flag_icone("en-AU"), "sm_flag_us"));
  assert(!strcmp(streammax_flag_icone("English (UK)"), "sm_flag_us"));
  assert(streammax_flag_icone("Dual Audio") == NULL);
  assert(streammax_flag_idioma_legivel("pt-br"));
  assert(!streammax_flag_idioma_legivel(" / / "));
  assert(!streammax_flag_idioma_legivel(" - "));
  assert(!streammax_flag_idioma_legivel(""));
  {
    const char *bandeiras[4] = {0};
    assert(streammax_audio_bandeiras("pt-PT,English,en-GB,pt-BR,es-MX", bandeiras, 4) == 3);
    assert(!strcmp(bandeiras[0], "sm_flag_br"));
    assert(!strcmp(bandeiras[1], "sm_flag_us"));
    assert(!strcmp(bandeiras[2], "sm_flag_mx"));
    assert(streammax_audio_bandeiras("", bandeiras, 4) == 0);
    assert(streammax_audio_bandeiras("Dual Audio,Subs: pt-BR", bandeiras, 4) == 0);
  }
  return 0;
}
