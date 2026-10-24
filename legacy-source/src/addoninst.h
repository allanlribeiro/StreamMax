#ifndef STREAMMAX_ADDONINST_H
#define STREAMMAX_ADDONINST_H
#include "addonurl.h"
#include <stdint.h>
typedef struct {
  uint64_t sessao;
  char url[NV_ADDON_URL_MAX], nome[64], erro[200];
  char *manifesto;
} AddonInstResultado;
/* Só valida no worker. Apenas a UI pode persistir no perfil capturado. */
int addoninst_iniciar(const char *url, uint64_t sessao);
void addoninst_cancelar(void);
int addoninst_rodando(void);
/* Estado atômico: 0 = aguardando, 1 = resultado transferido, -1 = cancelado/sem pedido. */
int addoninst_resultado(AddonInstResultado *saida);
void addoninst_liberar(AddonInstResultado *resultado);
#endif
