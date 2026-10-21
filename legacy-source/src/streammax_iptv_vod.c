// StreamMax IPTV VOD: filmes e series do Xtream separados dos catálogos.
#include "streammax_iptv_vod.h"
#include "xtream.h"
#include "perfisessao.h"
#include "streammax_livetv_columns.h"
#include "gfx.h"
#include "text.h"
#include "layout.h"
#include "logoapp.h"
#include "idioma.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VOD_CAT_CAP 48
#define VOD_ITEM_CAP 96
#define VOD_EP_CAP 96
#define VOD_LINHAS 9
#define VOD_LINHA_H 78.0f

typedef struct {
  int tipo; // 1 catalogo, 2 episodios
  int nCategorias, nItens, nEpisodios;
  XtreamCategoriaVod categorias[VOD_CAT_CAP];
  XtreamMidia itens[VOD_ITEM_CAP];
  XtreamEpisodio episodios[VOD_EP_CAP];
} VodResultado;

typedef struct {
  unsigned geracao;
  NvPerfilSessao sessao;
  int tipo, modo;
  char categoria[24], serie[80];
} VodTrabalho;

static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;
static _Atomic int fioAtivo;
static _Atomic unsigned geracao = 1;
static VodResultado *pendente;       // entregue pelo worker, lido no desenho
static VodResultado *visivel;        // somente fio de desenho
static int modo, carregando, erro, atualizarPendente;
static int focoCol, focoCategoria, focoItem, focoEp, serieAberta;
static StreamMaxVodPedido pedido;
static int pediu;

int streammax_iptv_modo(void) { return modo; }
int streammax_iptv_categoria_focada(void) { return focoCol == 0; }

static int itensNaCategoria(void) { return visivel ? visivel->nItens : 0; }

static void *buscar(void *p) {
  VodTrabalho *t = p;
  VodResultado *novo = calloc(1, sizeof *novo);
  if (novo && nv_perfil_sessao_atual(t->sessao)) {
    novo->tipo = t->tipo;
    if (t->tipo == 1) {
      // Categorias somente na primeira visita / sem filtro selecionado.
      // A lista do provedor nunca entra no catalogo dos addons/TMDB.
      if (!t->categoria[0])
        novo->nCategorias = xtream_vod_categorias(t->modo == 2, novo->categorias, VOD_CAT_CAP);
      novo->nItens = xtream_vod_listar_categoria(t->modo == 2,
                                 t->categoria[0] ? t->categoria : NULL,
                                 novo->itens, VOD_ITEM_CAP);
    } else {
      novo->nEpisodios = xtream_series_episodios(t->serie, novo->episodios, VOD_EP_CAP);
    }
  }
  pthread_mutex_lock(&trava);
  if (novo && nv_perfil_sessao_atual(t->sessao) &&
      t->geracao == atomic_load_explicit(&geracao, memory_order_acquire)) {
    free(pendente);
    pendente = novo;
    novo = NULL;
  }
  pthread_mutex_unlock(&trava);
  free(novo);
  free(t);
  atomic_store_explicit(&fioAtivo, 0, memory_order_release);
  return NULL;
}

static void solicitar(int tipo) {
  VodTrabalho *t;
  pthread_t fio;
  if (atomic_load_explicit(&fioAtivo, memory_order_acquire)) return;
  if (!xtream_configurado()) { carregando = 0; erro = 2; atualizarPendente = 0; return; }
  t = calloc(1, sizeof *t);
  if (!t) { erro = 1; atualizarPendente = 0; return; }
  t->geracao = atomic_load_explicit(&geracao, memory_order_acquire);
  t->sessao = nv_perfil_sessao_ler();
  t->tipo = tipo; t->modo = modo;
  if (tipo == 1 && focoCategoria > 0 && visivel &&
      focoCategoria - 1 < visivel->nCategorias)
    snprintf(t->categoria, sizeof t->categoria, "%s",
             visivel->categorias[focoCategoria - 1].id);
  if (tipo == 2 && visivel && focoItem < visivel->nItens)
    snprintf(t->serie, sizeof t->serie, "%s", visivel->itens[focoItem].id);
  if (tipo == 2 && !t->serie[0]) { free(t); return; }
  atomic_store_explicit(&fioAtivo, 1, memory_order_release);
  if (pthread_create(&fio, NULL, buscar, t)) {
    atomic_store_explicit(&fioAtivo, 0, memory_order_release);
    free(t); erro = 1; atualizarPendente = 0; return;
  }
  pthread_detach(fio);
  carregando = tipo;
  erro = 0;
  atualizarPendente = 0;
}

void streammax_iptv_sessao(void) {
  atomic_fetch_add_explicit(&geracao, 1, memory_order_acq_rel);
  pthread_mutex_lock(&trava);
  free(pendente); pendente = NULL;
  pthread_mutex_unlock(&trava);
  free(visivel); visivel = NULL;
  modo = carregando = erro = atualizarPendente = 0;
  focoCol = focoCategoria = focoItem = focoEp = serieAberta = 0;
  pediu = 0; memset(&pedido, 0, sizeof pedido);
}

void streammax_iptv_mudar(int novo) {
  if (novo < 0 || novo > 2 || novo == modo) return;
  atomic_fetch_add_explicit(&geracao, 1, memory_order_acq_rel);
  pthread_mutex_lock(&trava);
  free(pendente); pendente = NULL;
  pthread_mutex_unlock(&trava);
  free(visivel); visivel = NULL;
  modo = novo;
  carregando = erro = serieAberta = 0;
  focoCol = focoCategoria = focoItem = focoEp = 0;
  atualizarPendente = novo != 0;
  pediu = 0; memset(&pedido, 0, sizeof pedido);
}

void streammax_iptv_atualizar(void) {
  VodResultado *r = NULL;
  if (!modo) return;
  pthread_mutex_lock(&trava);
  if (pendente) { r = pendente; pendente = NULL; }
  pthread_mutex_unlock(&trava);
  if (r) {
    if (r->tipo == 2) {
      if (visivel) {
        memcpy(visivel->episodios, r->episodios, sizeof r->episodios);
        visivel->nEpisodios = r->nEpisodios;
      }
    } else {
      if (visivel && !r->nCategorias && focoCategoria > 0) {
        r->nCategorias = visivel->nCategorias;
        memcpy(r->categorias, visivel->categorias, sizeof r->categorias);
      }
      free(visivel); visivel = r; r = NULL;
      focoItem = focoEp = 0;
      serieAberta = 0;
    }
    free(r);
    carregando = 0;
    erro = visivel == NULL ? 1 : 0;
  }
  if (atualizarPendente && !carregando &&
      !atomic_load_explicit(&fioAtivo, memory_order_acquire))
    solicitar(atualizarPendente);
}

static void carregarCategoria(void) {
  atomic_fetch_add_explicit(&geracao, 1, memory_order_acq_rel);
  atualizarPendente = 1; carregando = 1;
  focoItem = focoEp = serieAberta = 0;
}

static void carregarSerie(void) {
  if (modo != 2 || !visivel || focoItem >= visivel->nItens) return;
  serieAberta = 1;
  focoCol = 2;
  focoEp = 0;
  visivel->nEpisodios = 0;
  atomic_fetch_add_explicit(&geracao, 1, memory_order_acq_rel);
  atualizarPendente = 2; carregando = 2;
}

static void pedirFilme(void) {
  XtreamMidia *m;
  if (!visivel || focoItem >= visivel->nItens) return;
  m = &visivel->itens[focoItem];
  memset(&pedido, 0, sizeof pedido);
  if (!xtream_vod_url(m, pedido.url, sizeof pedido.url)) return;
  snprintf(pedido.id, sizeof pedido.id, "%s", m->id);
  snprintf(pedido.nome, sizeof pedido.nome, "%s", m->nome);
  snprintf(pedido.poster, sizeof pedido.poster, "%s", m->poster);
  pediu = 1;
}
static void pedirEpisodio(void) {
  XtreamEpisodio *e;
  XtreamMidia *m;
  if (!visivel || !serieAberta || focoItem >= visivel->nItens ||
      focoEp >= visivel->nEpisodios) return;
  e = &visivel->episodios[focoEp];
  m = &visivel->itens[focoItem];
  memset(&pedido, 0, sizeof pedido);
  if (!xtream_episodio_url(e, pedido.url, sizeof pedido.url)) return;
  snprintf(pedido.id, sizeof pedido.id, "%s", e->id);
  snprintf(pedido.nome, sizeof pedido.nome, "%s - T%d E%d",
           m->nome, e->temporada, e->episodio);
  snprintf(pedido.poster, sizeof pedido.poster, "%s", m->poster);
  pedido.temporada = e->temporada;
  pedido.episodio = e->episodio;
  pediu = 1;
}

int streammax_iptv_pediu(StreamMaxVodPedido *out) {
  if (!pediu || !out) return 0;
  *out = pedido;
  memset(&pedido, 0, sizeof pedido); pediu = 0;
  return 1;
}

int streammax_iptv_evento(const SDL_Event *e) {
  SDL_Keycode k;
  int n = 0;
  if (!modo || e->type != SDL_KEYDOWN) return 0;
  k = e->key.keysym.sym;
  if (k == SDLK_LEFT) { if (focoCol > 0) focoCol--; return 1; }
  if (k == SDLK_RIGHT) {
    if (focoCol < (modo == 2 && serieAberta ? 2 : 1)) focoCol++;
    return 1;
  }
  if (k == SDLK_UP || k == SDLK_DOWN) {
    int d = k == SDLK_DOWN ? 1 : -1;
    if (!focoCol) {
      n = 1 + (visivel ? visivel->nCategorias : 0);
      focoCategoria = streammax_tv_limitar(focoCategoria + d, n);
    } else if (focoCol == 1) {
      focoItem = streammax_tv_limitar(focoItem + d, itensNaCategoria());
      serieAberta = 0; focoEp = 0;
    } else {
      focoEp = streammax_tv_limitar(focoEp + d, visivel ? visivel->nEpisodios : 0);
    }
    return 1;
  }
  if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
    if (!carregando && visivel) {
      if (!focoCol) carregarCategoria();
      else if (focoCol == 1) {
        if (modo == 2) carregarSerie();
        else pedirFilme();
      } else pedirEpisodio();
    }
    return 1;
  }
  return 1;
}

static void rotulo(const char *texto, float x, float y, float w, int ativo, float a) {
  TxtLinha t = txt_linha_corta(TXT_PG_ROTULO, texto,
                       ativo ? 248 : 186, ativo ? 247 : 206,
                       ativo ? 255 : 225, 255, w);
  txt_desenhar_alpha(t, x, y, a);
}
static void painel(GfxRect r, float a) {
  gfx_cor(r, 0.018f, 0.018f, 0.044f, 0.087f, a);
}
static void faixa(GfxRect r, int foco, float a) {
  gfx_cor(r, 0.10f, foco ? 0.018f : 0.016f,
          foco ? 0.20f : 0.050f, foco ? 0.42f : 0.095f, a);
  if (foco)
    gfx_cor((GfxRect){r.x,r.y,4.0f,r.h},0,0.08f,0.70f,1.0f,a);
}
void streammax_iptv_abas(float a, int foco) {
  const char *tit[3] = {"Canais ao vivo", "Filmes IPTV", "Séries IPTV"};
  const float x = NV_MARGEM_X + 332.0f, y = 112.0f, w = 248.0f, gap = 12.0f;
  for (int j = 0; j < 3; j++) {
    GfxRect r = {x+j*(w+gap),y,w,32.0f};
    int sel = j == modo;
    faixa(r, sel || (foco == j), a);
    rotulo(tit[j],r.x+13.0f,r.y+5.0f,r.w-26.0f,sel,a);
  }
}

void streammax_iptv_desenhar(float a, int focoAba) {
  const float x = NV_MARGEM_X, y = 173.0f, gap = 18.0f;
  const float catW = 300.0f, itemW = 624.0f;
  const float itemX = x + catW + gap, detX = itemX + itemW + gap;
  const float detW = NV_TELA_W - NV_MARGEM_X - detX;
  const float ry = y + 55.0f, rh = VOD_LINHA_H - 6.0f;
  const int firstCat = streammax_tv_primeira(focoCategoria,VOD_LINHAS,visivel ? visivel->nCategorias+1 : 1);
  const int firstItem = streammax_tv_primeira(focoItem,VOD_LINHAS,visivel ? visivel->nItens : 0);
  const int firstEp = streammax_tv_primeira(focoEp,6,visivel ? visivel->nEpisodios : 0);
  GfxRect full = {0,0,NV_TELA_W,NV_TELA_H};
  char titulo[200], info[220];
  gfx_cor(full,0,0.007f,0.018f,0.039f,a);
  gfx_cor((GfxRect){0,0,NV_TELA_W,156.0f},0,0.012f,0.034f,0.079f,a);
  logoapp_streammax_wordmark((GfxRect){x+18,36,246,44},a);
  rotulo(modo==1 ? "FILMES IPTV" : "SÉRIES IPTV",x+358,40,440,1,a);
  streammax_iptv_abas(a, focoAba);
  painel((GfxRect){x,y,catW,823},a);
  painel((GfxRect){itemX,y,itemW,823},a);
  painel((GfxRect){detX,y,detW,823},a);
  rotulo("Categorias",x+18,y+15,catW-40,1,a);
  rotulo(modo==1?"Filmes do provedor":"Séries do provedor",itemX+18,y+15,itemW-40,1,a);
  rotulo(modo==1?"Detalhes":"Episódios / detalhes",detX+18,y+15,detW-40,1,a);
  for(int j=0;j<VOD_LINHAS;j++) {
    int ci = firstCat+j;
    GfxRect r = {x+9,ry+j*VOD_LINHA_H,catW-18,rh};
    if (ci> (visivel ? visivel->nCategorias : 0)) break;
    faixa(r,ci==focoCategoria && focoCol==0,a);
    rotulo(ci==0?"Todas as categorias":visivel->categorias[ci-1].nome,
           r.x+13,r.y+22,r.w-25,ci==focoCategoria,a);
  }
  if (visivel) for (int j=0;j<VOD_LINHAS && firstItem+j<visivel->nItens;j++) {
    int i=firstItem+j;
    GfxRect r={itemX+9,ry+j*VOD_LINHA_H,itemW-18,rh};
    faixa(r,i==focoItem&&focoCol==1,a);
    rotulo(visivel->itens[i].nome,r.x+18,r.y+16,r.w-32,i==focoItem,a);
    if (visivel->itens[i].ano>0) {
      snprintf(info,sizeof info,"%d · IPTV",visivel->itens[i].ano);
      TxtLinha meta=txt_linha_corta(TXT_DET_META2,info,140,174,218,255,r.w-36);
      txt_desenhar_alpha(meta,r.x+19,r.y+43,a);
    }
  }
  if (carregando || atualizarPendente) {
    snprintf(info,sizeof info,"%s",i18n("Carregando conteúdo do provedor…"));
  } else if (erro==2) snprintf(info,sizeof info,"%s",i18n("Configure uma lista Xtream em Ajustes."));
  else if(erro) snprintf(info,sizeof info,"%s",i18n("Não foi possível consultar o provedor."));
  else if (!visivel || !visivel->nItens) snprintf(info,sizeof info,"%s",i18n("Nenhum título nesta categoria."));
  else snprintf(info,sizeof info,"%d título(s) neste resultado · até %d por consulta",
                visivel->nItens,VOD_ITEM_CAP);
  TxtLinha status=txt_linha_corta(TXT_DET_META2,info,148,186,225,255,detW-40);
  txt_desenhar_alpha(status,detX+18,ry+7,a);
  if(visivel&&focoItem<visivel->nItens) {
    XtreamMidia *m=&visivel->itens[focoItem];
    snprintf(titulo,sizeof titulo,"%s",m->nome);
    TxtLinha nm=txt_linha_corta(TXT_G26B,titulo,240,247,255,255,detW-38);
    txt_desenhar_alpha(nm,detX+18,ry+74,a);
    snprintf(info,sizeof info,"%s",modo==1?"OK para reproduzir":"OK na série para carregar os episódios");
    rotulo(info,detX+18,ry+122,detW-38,0,a);
    if(modo==2 && serieAberta) {
      rotulo("Episódios",detX+18,ry+175,detW-38,1,a);
      for(int j=0;j<6&&firstEp+j<visivel->nEpisodios;j++) {
        int i=firstEp+j;
        XtreamEpisodio *ep=&visivel->episodios[i];
        GfxRect r={detX+10,ry+215+j*VOD_LINHA_H,detW-20,rh};
        faixa(r,i==focoEp&&focoCol==2,a);
        snprintf(info,sizeof info,"T%d  E%d   %s",ep->temporada,ep->episodio,ep->titulo);
        rotulo(info,r.x+15,r.y+23,r.w-30,i==focoEp,a);
      }
    }
  }
  rotulo("← → alternar colunas  ·  OK selecionar  ·  Azul: outra área  ·  Voltar: TV ao vivo",
         x,NV_TELA_H-45,NV_TELA_W-2*x,0,a);
}
