#include "../src/streammax_ui_brand.h"
#include <assert.h>
#include <string.h>

int main(void) {
  assert(!strcmp(streammax_nome_visivel("Nuvio"), "StreamMax"));
  assert(!strcmp(streammax_nome_visivel("NUVIO"), "STREAMMAX"));
  assert(!strcmp(streammax_nome_visivel("nuvio"), "streammax"));
  assert(!strcmp(streammax_nome_visivel("Conta Nuvio"), "Conta StreamMax"));
  assert(!strcmp(streammax_nome_visivel("Nuvio Legacy"), "StreamMax Legacy"));
  assert(!strcmp(streammax_nome_visivel("Nenhuma fonte"), "Nenhuma fonte"));
  assert(!strcmp(streammax_nome_visivel(NULL) == NULL ? "ok" : "erro", "ok"));
  return 0;
}
