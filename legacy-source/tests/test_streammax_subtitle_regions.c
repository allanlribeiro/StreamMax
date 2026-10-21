// Teste puro de idiomas: sem SDL, video ou rede. Executar com:
// cc -std=c11 -Wall -Wextra -I src src/linguas.c tests/test_streammax_subtitle_regions.c -o /tmp/test_regions && /tmp/test_regions
#include "../src/linguas.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int igual(const char *a, const char *b) { return !strcmp(a, b); }

int main(void) {
  const char *pt[] = {"pt-pt", "pob", "por"};
  const char *ptBr[] = {"pt-pt", "pob", "por"};
  const char *semRegiao[] = {"por", "pt", "eng"};
  assert(igual(ling_legenda_chave("pob", NULL), "pt-br"));
  assert(igual(ling_legenda_chave("pt_BR", NULL), "pt-br"));
  assert(igual(ling_legenda_chave("pt-PT", NULL), "pt-pt"));
  assert(igual(ling_legenda_chave("por", "Portuguese (Portugal)"), "pt-pt"));
  assert(igual(ling_legenda_chave("pt", "European Portuguese"), "pt-pt"));
  assert(igual(ling_legenda_chave("por", "Português (Brasil)"), "pt-br"));
  assert(igual(ling_legenda_chave("por", "Filme.1080p.pt-PT.srt"), "pt-pt"));
  assert(igual(ling_legenda_chave("por", "Sem indicacao de regiao"), "pt"));
  assert(igual(ling_legenda_chave("eng", "Portuguese (Portugal)"), "eng"));
  assert(igual(ling_do_nome("Portuguese (Portugal)"), "pt-pt"));
  assert(igual(ling_do_nome("Brazilian Portuguese"), "pob"));
  assert(igual(ling_nome("pt-pt"), "Português (Portugal)"));
  assert(igual(ling_nome("pt-br"), "Português (Brasil)"));
  assert(igual(ling_selo("pt-pt"), "PT-PT"));
  assert(igual(ling_selo("pt-br"), "PT-BR"));
  assert(ling_legenda_mesma_regiao("pob", "pt-BR"));
  assert(!ling_legenda_mesma_regiao("pob", "pt-PT"));
  assert(!ling_legenda_mesma_regiao("pt", "pt-BR"));
  assert(!ling_legenda_mesma_regiao("pt", "pt-PT"));
  assert(ling_legenda_mesma_regiao("por", "pt"));
  assert(ling_casa("pob", "pt-pt")); // outros usos mantem fallback por familia

  // Preferencia especifica nao liga automaticamente a variante oposta.
  assert(ling_legenda_auto("pt-pt", pt, 3, 1, NULL, 0, 1) == 0);
  assert(ling_legenda_auto("pt-br", ptBr, 3, 1, NULL, 0, 1) == 1);
  assert(ling_legenda_auto("pt-br", ptBr, 1, 1, NULL, 0, 1) == LING_AUTO_NADA);
  assert(ling_legenda_auto("pt-br", semRegiao, 3, 1, NULL, 0, 1) == 0);
  assert(ling_afinidade("pt-pt", "pt-pt") > ling_afinidade("pt-br", "pt-pt"));
  assert(ling_afinidade("pt-br", "pt-br") > ling_afinidade("pt-pt", "pt-br"));
  puts("OK: PT-BR, PT-PT e portugues generico separados sem confundir audio");
  return 0;
}
