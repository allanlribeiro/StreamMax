// StreamMax TV ao vivo: geometria e navegacao da tela de tres colunas.
// Independente de SDL, rede e do player. Decide foco/espera; o app inicia streams.
#ifndef STREAMMAX_LIVETV_COLUMNS_H
#define STREAMMAX_LIVETV_COLUMNS_H

#define SM_TV_COL_CATEGORIAS 0
#define SM_TV_COL_CANAIS 1
#define SM_TV_CAT_W 300.0f
#define SM_TV_CANAL_W 580.0f
#define SM_TV_COL_GAP 18.0f
#define SM_TV_ROW_H 72.0f
#define SM_TV_TITULO_Y 45.0f
#define SM_TV_BODY_Y 148.0f
#define SM_TV_BOTTOM_MARGIN 82.0f
#define SM_TV_VISIBLE_ROWS 11
// A mudanca de canal espera o foco repousar, sem disparar N fontes ao segurar a seta.
#define SM_TV_PREVIEW_FOCO_MS 500u

// Na tela StreamMax o canal focado deve atualizar o preview ate depois de
// voltar da tela cheia. No guia classico, preservar a politica de "canal preso".
static inline int streammax_tv_preview_segue_foco(int tresColunas,
                                                  int colunaFoco,
                                                  int previewPreso) {
  return tresColunas ? colunaFoco == SM_TV_COL_CANAIS : !previewPreso;
}


static inline int streammax_tv_limitar(int v, int total) {
  if (total <= 0) return 0;
  return v < 0 ? 0 : v >= total ? total - 1 : v;
}
static inline int streammax_tv_primeira(int selecionado, int visiveis, int total) {
  int primeiro, maximo;
  if (visiveis < 1 || total <= visiveis) return 0;
  primeiro = selecionado - visiveis / 2;
  maximo = total - visiveis;
  return primeiro < 0 ? 0 : primeiro > maximo ? maximo : primeiro;
}
static inline float streammax_tv_progresso(long long agora, long long inicio, long long fim) {
  float fracao;
  if (fim <= inicio) return 0.0f;
  fracao = (float)(agora - inicio) / (float)(fim - inicio);
  return fracao < 0.0f ? 0.0f : fracao > 1.0f ? 1.0f : fracao;
}
#endif
