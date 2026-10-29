// Sincronizacao manual assistida: documento imutavel, sem rede nem player.
#ifndef NV_LEGFALA_H
#define NV_LEGFALA_H
#include "legenda.h"
#define LEG_FALA_REACAO_MS 300
typedef struct {
  LegendaDocumento *doc;
  unsigned geracao;
  int *indices, n, foco, capturada;
  double capturaMs;
} LegFala;
void legfala_fechar(LegFala *f);
int legfala_abrir(LegFala *f, LegendaDocumento *doc, unsigned geracao);
int legfala_capturar(LegFala *f, double videoMs, double legendaMs);
int legfala_mover(LegFala *f, int passo);
const LegendaCue *legfala_cue(const LegFala *f, int indice);
void legfala_frase(const LegendaCue *cue, char *dst, size_t tam);
// Atraso positivo = aparece depois. Recusa alem de +/-60 s, sem clamp oculto.
int legfala_calcular(const LegFala *f, int *atrasoMs);
#endif
