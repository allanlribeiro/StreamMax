// Executa os ajustes reais com preferencias antigas ativadas, sem janela/GPU.
#include "../src/ajustes.c"
#include "../src/salto.h"
#include <assert.h>
int main(void) {
  const int antigas[]={AJ_SEEKR_LIGADO,AJ_SEEKR_FITA,AJ_SEEKR_AJUSTE,AJ_SEEKR_CHAVE,AJ_SEEKR_TESTAR};
  strcpy(seekrChave,"fixture-antiga");
  valor[AJ_SEEKR_LIGADO]=0;valor[AJ_SEEKR_FITA]=0;valor[AJ_SEEKR_AJUSTE]=15;
  assert(!ajustes_seekr_ligado()&&!ajustes_seekr_habilitado()&&!ajustes_seekr_fita());
  for(unsigned i=0;i<sizeof antigas/sizeof *antigas;i++)assert(!ajOpcaoDisponivel(antigas[i]));
  assert(ajOpcaoDisponivel(AJ_CACHE_SEEK));
  SaltoEst st={0};
  assert(salto_tecla(&st,1,0,1000,7200)==10.0f);
  assert(salto_tecla(&st,1,0,1200,7200)==10.0f);
  assert(salto_fim_ms(0)==420u);
  puts("OK: preferencias antigas nao reativam frames; cache e salto normal continuam disponiveis");
}
