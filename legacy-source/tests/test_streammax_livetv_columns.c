#include "../src/streammax_livetv_columns.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
  assert(streammax_tv_limitar(-5, 30) == 0);
  assert(streammax_tv_limitar(18, 30) == 18);
  assert(streammax_tv_limitar(100, 30) == 29);
  assert(streammax_tv_limitar(12, 0) == 0);
  assert(streammax_tv_primeira(0, 11, 40) == 0);
  assert(streammax_tv_primeira(8, 11, 40) == 3);
  assert(streammax_tv_primeira(38, 11, 40) == 29);
  assert(streammax_tv_primeira(2, 11, 3) == 0);
  assert(streammax_tv_primeira(0, 0, 30) == 0);
  assert(streammax_tv_progresso(10, 20, 20) == 0.0f);
  assert(streammax_tv_progresso(9, 10, 30) == 0.0f);
  assert(streammax_tv_progresso(20, 10, 30) == 0.5f);
  assert(streammax_tv_progresso(45, 10, 30) == 1.0f);
  // O modo StreamMax segue o foco (na coluna de canais), mesmo apos
  // voltar da tela cheia com uma sessao de video ja em andamento.
  assert(SM_TV_PREVIEW_FOCO_MS >= 350u && SM_TV_PREVIEW_FOCO_MS <= 700u);
  assert(streammax_tv_preview_segue_foco(1, SM_TV_COL_CANAIS, 0) == 1);
  assert(streammax_tv_preview_segue_foco(1, SM_TV_COL_CANAIS, 1) == 1);
  assert(streammax_tv_preview_segue_foco(1, SM_TV_COL_CATEGORIAS, 0) == 0);
  assert(streammax_tv_preview_segue_foco(1, SM_TV_COL_CATEGORIAS, 1) == 0);
  // O guia classico mantem seu comportamento anterior.
  assert(streammax_tv_preview_segue_foco(0, SM_TV_COL_CATEGORIAS, 0) == 1);
  assert(streammax_tv_preview_segue_foco(0, SM_TV_COL_CANAIS, 1) == 0);
  puts("OK: geometria, EPG e politica de previa automatica do guia StreamMax");
  return 0;
}
