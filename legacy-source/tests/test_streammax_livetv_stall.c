// Contrato do watchdog leve (sem player, SDL, rede nem credenciais).
// cc -std=gnu11 -Wall -Wextra -Werror tests/test_streammax_livetv_stall.c -o /tmp/live-stall
#include "../src/livestall.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
  NvLiveStall s;
  nv_ls_zerar(&s);
  assert(nv_ls_passo_buf(&s, 1000u, 0.0, 1, 0, 0, 0, 0.0) == NV_LS_NADA);
  assert(nv_ls_passo_buf(&s, 2000u, 5.0, 1, 0, 0, 0, 0.0) == NV_LS_NADA);
  // Nao reiniciar em queda momentanea de 3 s.
  assert(nv_ls_passo_buf(&s, 5000u, 5.0, 1, 0, 0, 3000, 0.0) == NV_LS_NADA);
  assert(nv_ls_passo_buf(&s, 11001u, 5.0, 1, 0, 0, 9000, 0.0) == NV_LS_AGENDOU);
  assert(s.tentativa == 1 && s.pendente == 1);
  assert(nv_ls_passo_buf(&s, 11500u, 5.0, 1, 0, 0, 0, 0.0) == NV_LS_ESPERA);
  assert(nv_ls_passo_buf(&s, 12002u, 5.0, 1, 0, 0, 0, 0.0) == NV_LS_REABRIR);
  nv_ls_reaberto(&s, 12002u);
  assert(!s.pendente && s.tentativa == 1);
  // Pausa feita pelo usuario nao e travamento.
  assert(nv_ls_passo_buf(&s, 13000u, 7.0, 1, 0, 0, 0, 0.0) == NV_LS_NADA);
  assert(nv_ls_passo_buf(&s, 25000u, 7.0, 1, 1, 0, 0, 0.0) == NV_LS_NADA);
  assert(nv_ls_passo_buf(&s, 25010u, 7.0, 1, 0, 0, 0, 0.0) == NV_LS_NADA);

  // Falta de segmentos: nao confundir com a espera por um novo segmento.
  nv_ls_zerar(&s);
  assert(nv_ls_passo_buf(&s, 1000u, 90.0, 1, 0, 0, 0, 100.0) == NV_LS_NADA);
  assert(nv_ls_passo_buf(&s, 5000u, 98.0, 1, 0, 0, 0, 100.0) == NV_LS_NADA);
  assert(nv_ls_passo_buf(&s, 17001u, 99.0, 1, 0, 0, 0, 100.0) == NV_LS_AGENDOU);
  assert(s.fonteSeca);
  // Segmentos voltaram ANTES de executar reconexao: cancela corretamente.
  assert(nv_ls_passo_buf(&s, 18010u, 99.1, 1, 0, 0, 0, 108.0) == NV_LS_NADA);
  assert(!s.pendente && s.tentativa == 0);

  puts("OK: live stall aguarda, reconecta quando necessario e respeita pausa");
  return 0;
}
