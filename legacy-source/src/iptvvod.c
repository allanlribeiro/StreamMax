// Browser VOD Xtream: somente uma consulta em voo, listas virtualizadas,
// capas sob demanda e nenhuma reproducao enquanto o foco apenas navega.
#include "iptvvod.h"
#include "xtream.h"
#include "xtreamvod.h"
#include "streammax_livetv_columns.h"
#include "gfx.h"
#include "text.h"
#include "tex_cache.h"
#include "layout.h"
#include "logoapp.h"
#include "ponteiro.h"
#include "teclado.h"
#include "buscanorm.h"
#include "dados.h"
#include "perfis.h"
#include "anim.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { V_CAPACIDADES, V_LISTA, V_EPISODIOS };
typedef struct {
  int trabalho, tipo, n[2], erro[2], total;
  unsigned geracao;
  NvPerfilSessao sessao;
  uint64_t revisao;
  char categoria[32], serie[32];
  XtreamVodCategoria cats[2][XV_MAX_CATEGORIAS];
  XtreamVodItem *itens;
} VodCarga;
static XtreamVodCategoria cats[2][XV_MAX_CATEGORIAS];
static int nCats[2], disponivel[2], capacidadePronta;
static XtreamVodItem *itens, *pais;
static int nItens, nPais, totalItens, erroLista;
static int aba, abaFoco, focoAbas, coluna = 1, linha, categoria, categoriaCarregada = -1;
static int menuLinha, acao;
static int modoEpisodios, temporadas[128], nTemporadas, linhaSerie;
static XtreamVodItem serieAtual;
static int indices[XV_MAX_ITENS], nVisiveis;
static unsigned geracao = 1;
static uint64_t revisao;
static NvPerfilSessao sessao;
static int rodando, desejado, pararPreview, tocar, buscaPendente, soFavoritos;
static Uint32 categoriaDesde;
static char busca[96], favoritos[500][64];
static int nFavoritos;
static float animLinha, animCategoria, transicao = 1;
static VodCarga *resultado;
static atomic_int pronto;

static void *carregar(void *arg) {
  VodCarga *c = arg;
  if (c->trabalho == V_CAPACIDADES) {
    for (int i = 0; i < 2; i++) {
      int n = xtream_vod_categorias(i + 1, c->cats[i], XV_MAX_CATEGORIAS, c->sessao, c->revisao);
      c->erro[i] = n < 0 ? n : 0;
      c->n[i] = n > 0 ? n : 0;
      // Catalogo sem categorias tambem e VOD. Apenas um item para sondar.
      if (n == 0) {
        XtreamVodItem item;
        n = xtream_vod_itens(i + 1, "", &item, 1, NULL, c->sessao, c->revisao);
        if (n > 0) {
          snprintf(c->cats[i][0].nome, sizeof c->cats[i][0].nome, "Todos");
          c->n[i] = 1;
        } else if (n < 0) c->erro[i] = n;
      }
    }
  } else {
    c->itens = calloc(XV_MAX_ITENS, sizeof *c->itens);
    int n = c->itens ? (c->trabalho == V_EPISODIOS
      ? xtream_vod_episodios(c->serie, c->itens, XV_MAX_ITENS, &c->total, c->sessao, c->revisao)
      : xtream_vod_itens(c->tipo, c->categoria, c->itens, XV_MAX_ITENS, &c->total, c->sessao, c->revisao)) : XV_REDE;
    c->erro[0] = n < 0 ? n : 0;
    c->n[0] = n > 0 ? n : 0;
    if (n > 0) {
      XtreamVodItem *menor = realloc(c->itens, (size_t)n * sizeof *menor);
      if (menor) c->itens = menor;
    } else { free(c->itens); c->itens = NULL; }
  }
  resultado = c;
  atomic_store_explicit(&pronto, 1, memory_order_release);
  return NULL;
}
static void iniciar(int trabalho) {
  if (rodando) return;
  VodCarga *c = calloc(1, sizeof *c);
  if (!c) { erroLista = XV_REDE; desejado = 0; return; }
  c->trabalho = trabalho; c->tipo = aba; c->sessao = sessao;
  c->revisao = revisao; c->geracao = geracao;
  if (aba && nCats[aba - 1] && categoria < nCats[aba - 1])
    snprintf(c->categoria, sizeof c->categoria, "%s", cats[aba - 1][categoria].id);
  snprintf(c->serie, sizeof c->serie, "%s", serieAtual.id);
  rodando = 1; desejado = 0;
  pthread_t fio;
  if (pthread_create(&fio, NULL, carregar, c)) { free(c); rodando = 0; erroLista = XV_REDE; return; }
  pthread_detach(fio);
}
static void nomeFav(char *out, size_t n) { snprintf(out, n, "iptv-vod-fav-p%d.txt", perfis_ativo()); }
static void favLer(void) {
  char arq[64]; nomeFav(arq, sizeof arq); char *s = dados_ler(arq);
  nFavoritos = 0;
  if (!s) return;
  for (char *p = s; *p && nFavoritos < 500;) {
    char *f = strchr(p, '\n'); if (f) *f++ = 0;
    if (!strncmp(p, "xtv:", 4) && strlen(p) < 64)
      snprintf(favoritos[nFavoritos++], sizeof favoritos[0], "%s", p);
    if (!f) break;
    p = f;
  }
  free(s);
}
static int favorito(const XtreamVodItem *it) {
  char id[64];
  if (!xtream_vod_identidade(modoEpisodios ? XV_EPISODIO : aba, it->id, id, sizeof id)) return 0;
  for (int i = 0; i < nFavoritos; i++) if (!strcmp(id, favoritos[i])) return 1;
  return 0;
}
static void filtrar(void) {
  char alvo[192], nome[320]; nVisiveis = 0;
  busca_normalizar(busca, alvo, sizeof alvo);
  for (int i = 0; i < nItens; i++) {
    if (modoEpisodios && nTemporadas && itens[i].temporada != temporadas[categoria]) continue;
    if (soFavoritos && !favorito(&itens[i])) continue;
    busca_normalizar(itens[i].nome, nome, sizeof nome);
    if (*alvo && !strstr(nome, alvo)) continue;
    indices[nVisiveis++] = i;
  }
  linha = streammax_tv_limitar(linha, nVisiveis);
}
static XtreamVodItem *selecionado(void) {
  return linha >= 0 && linha < nVisiveis ? &itens[indices[linha]] : NULL;
}
static void favAlternar(void) {
  XtreamVodItem *it = selecionado(); char id[64], arq[64];
  if (!it || !xtream_vod_identidade(modoEpisodios ? XV_EPISODIO : aba, it->id, id, sizeof id)) return;
  int i;
  for (i = 0; i < nFavoritos; i++) if (!strcmp(id, favoritos[i])) break;
  if (i < nFavoritos) { memmove(favoritos[i], favoritos[i+1], (size_t)(nFavoritos-i-1)*sizeof favoritos[0]); nFavoritos--; }
  else if (nFavoritos < 500) snprintf(favoritos[nFavoritos++], sizeof favoritos[0], "%s", id);
  char *txt = calloc(500, 65);
  if (txt) {
    size_t k = 0;
    for (i = 0; i < nFavoritos; i++) k += (size_t)snprintf(txt + k, 32500-k, "%s\n", favoritos[i]);
    nomeFav(arq, sizeof arq); dados_gravar(arq, txt); free(txt);
  }
  filtrar();
}
void iptvvod_abrir_sessao(void) {
  geracao++; free(itens); free(pais); itens = pais = NULL;
  nItens = nPais = nVisiveis = totalItens = erroLista = 0;
  memset(nCats, 0, sizeof nCats); memset(disponivel, 0, sizeof disponivel);
  aba = abaFoco = focoAbas = capacidadePronta = desejado = 0;
  modoEpisodios = categoria = linha = nTemporadas = soFavoritos = buscaPendente = tocar = 0;
  menuLinha = acao = 0;
  categoriaCarregada = -1; revisao = 0; busca[0] = 0; nFavoritos = 0;
  transicao = 1; animLinha = animCategoria = 0;
}
void iptvvod_preparar(void) {
  uint64_t r = xtream_revisao(); NvPerfilSessao s = nv_perfil_sessao_ler();
  if (r != revisao || s != sessao) {
    iptvvod_abrir_sessao(); sessao = s; revisao = r; favLer();
  }
  if (!capacidadePronta && !rodando && xtream_configurado()) iniciar(V_CAPACIDADES);
}
int iptvvod_ativo(void) { return aba != 0; }
int iptvvod_tem_abas(void) { return xtream_configurado(); }
void iptvvod_focar_abas(void) { focoAbas = 1; abaFoco = aba; }
void iptvvod_ao_vivo(void) { aba = abaFoco = focoAbas = tocar = 0; geracao++; desejado = 0; }
int iptvvod_parar_preview(void) { int p = pararPreview; pararPreview = 0; return p; }

static void escolherAba(int nova) {
  if (nova == aba) return;
  free(itens); free(pais); itens = pais = NULL;
  nItens = nPais = nVisiveis = totalItens = 0; modoEpisodios = 0;
  aba = abaFoco = nova; geracao++; erroLista = 0;
  categoria = linha = 0; coluna = 1; categoriaCarregada = -1;
  soFavoritos = 0; busca[0] = 0;
  desejado = nova != 0; categoriaDesde = SDL_GetTicks();
  animLinha = animCategoria = 0; transicao = 0;
  if (nova) pararPreview = 1;
}
static void voltarSeries(void) {
  if (!modoEpisodios) return;
  geracao++; desejado = 0; free(itens); itens = pais; pais = NULL;
  nItens = nPais; nPais = 0; nVisiveis = 0; modoEpisodios = 0;
  categoria = categoriaCarregada; linha = linhaSerie; erroLista = 0; filtrar();
}
static void confirmarItem(void) {
  XtreamVodItem *it = selecionado();
  if (!it || desejado || (rodando && !nItens)) return;
  if (aba == XV_SERIES && !modoEpisodios) {
    serieAtual = *it; pais = itens; nPais = nItens; itens = NULL;
    linhaSerie = linha; nItens = nVisiveis = nTemporadas = 0;
    modoEpisodios = 1; geracao++; categoria = linha = 0; erroLista = 0;
    desejado = 1; categoriaDesde = SDL_GetTicks();
  } else tocar = 1;
}
static void buscar(void) {
  teclado_abrir_com("Buscar no VOD", "Pesquisar nesta categoria", 80,
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -.,!?áàãâéêíóôõúçÁÀÃÂÉÊÍÓÔÕÚÇ", busca);
  teclado_permitir_vazio(1); teclado_aceitar_utf8(1); buscaPendente = 1;
}
static int totalCategorias(void) { return modoEpisodios ? nTemporadas : aba ? nCats[aba-1] : 0; }
static void selecionarCategoria(int i) {
  i = streammax_tv_limitar(i, totalCategorias());
  if (i == categoria) return;
  categoria = i; linha = 0;
  if (modoEpisodios) filtrar();
  else {
    geracao++; desejado = 1; categoriaDesde = SDL_GetTicks();
    nVisiveis = 0; erroLista = 0;
  }
}
int iptvvod_evento(const SDL_Event *e) {
  if (buscaPendente && teclado_aberto()) { teclado_evento(e); return 1; }
  if (e->type != SDL_KEYDOWN) return aba || focoAbas;
  SDL_Keycode k = e->key.keysym.sym;
  if (k == SDLK_TAB && iptvvod_tem_abas()) { iptvvod_focar_abas(); return 1; }
  if (focoAbas) {
    if (k == SDLK_LEFT || k == SDLK_RIGHT) {
      int dir = k == SDLK_RIGHT ? 1 : -1, proxima = abaFoco + dir;
      while (proxima >= 0 && proxima <= 2 && proxima && capacidadePronta && disponivel[proxima-1] == 0) proxima += dir;
      if (proxima >= 0 && proxima <= 2) { escolherAba(proxima); focoAbas = 1; }
    } else if (k == SDLK_DOWN || k == SDLK_RETURN || k == SDLK_KP_ENTER) focoAbas = 0;
    else if (k == SDLK_ESCAPE || k == SDLK_AC_BACK || k == SDLK_BACKSPACE) focoAbas = 0;
    return 1;
  }
  if (!aba) return 0;
  if (k == SDLK_ESCAPE || k == SDLK_AC_BACK || k == SDLK_BACKSPACE || e->key.keysym.scancode == NV_SCANCODE_BACK) {
    if (modoEpisodios) voltarSeries();
    else { escolherAba(0); focoAbas = 0; }
  } else if (k == SDLK_F5) {
    geracao++; desejado = 1; erroLista = 0; categoriaDesde = 0;
    if (!nCats[aba-1]) { capacidadePronta = 0; desejado = 0; }
  } else if (k == SDLK_f) favAlternar();
  else if (k == SDLK_SLASH || k == SDLK_s) buscar();
  else if (k == SDLK_LEFT) { if (coluna > 0) coluna--; }
  else if (k == SDLK_RIGHT) { if (coluna < 2) coluna++; }
  else if (k == SDLK_UP || k == SDLK_DOWN) {
    int d = k == SDLK_DOWN ? 1 : -1;
    if (coluna == 0) {
      // -3 buscar, -2 favoritos, -1 atualizar, depois categorias/temporadas.
      if (d < 0 && categoria == 0) { coluna = -1; linha = 0; }
      else selecionarCategoria(categoria + d);
    } else if (coluna == -1) {
      if (d < 0 && menuLinha == 0) iptvvod_focar_abas();
      else if (d > 0 && menuLinha == 2) coluna = 0;
      else menuLinha = streammax_tv_limitar(menuLinha + d, 3);
    } else if (coluna == 1) {
      if (d < 0 && linha == 0) iptvvod_focar_abas();
      else linha = streammax_tv_limitar(linha + d, nVisiveis);
    } else acao = streammax_tv_limitar(acao + d, 2);
  } else if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
    if (coluna == -1) {
      if (menuLinha == 0) buscar();
      else if (menuLinha == 1) { soFavoritos = !soFavoritos; linha = 0; filtrar(); }
      else { geracao++; desejado = 1; erroLista = 0; categoriaDesde = 0; }
    } else if (coluna == 0) coluna = 1;
    else if (coluna == 2 && acao == 1) favAlternar();
    else confirmarItem();
  }
  return 1;
}
void iptvvod_atualizar(float dt, Uint32 agora) {
  if (buscaPendente && teclado_aberto()) teclado_atualizar(dt, agora);
  if (atomic_load_explicit(&pronto, memory_order_acquire)) {
    VodCarga *c = resultado;
    atomic_store_explicit(&pronto, 0, memory_order_relaxed); rodando = 0;
    int atual = c->sessao == sessao && c->revisao == revisao &&
      nv_perfil_sessao_atual(c->sessao) && xtream_revisao() == c->revisao;
    if (atual && c->trabalho == V_CAPACIDADES) {
      memcpy(cats, c->cats, sizeof cats); memcpy(nCats, c->n, sizeof nCats);
      for (int i = 0; i < 2; i++) disponivel[i] = c->n[i] > 0 ? 1 : c->erro[i] ? -1 : 0;
      capacidadePronta = 1;
    } else if (atual && c->geracao == geracao) {
      free(itens); itens = c->itens; c->itens = NULL;
      nItens = c->n[0]; totalItens = c->total; erroLista = c->erro[0];
      linha = 0; transicao = 0;
      if (modoEpisodios) {
        nTemporadas = 0;
        for (int i = 0; i < nItens && nTemporadas < 128; i++) {
          if (!nTemporadas || temporadas[nTemporadas-1] != itens[i].temporada)
            temporadas[nTemporadas++] = itens[i].temporada;
          if (!itens[i].capa[0]) snprintf(itens[i].capa, sizeof itens[i].capa, "%s", serieAtual.capa);
        }
      } else categoriaCarregada = categoria;
      filtrar();
    }
    free(c->itens); free(c);
  }
  if (buscaPendente && !teclado_aberto()) {
    int r = teclado_resultado(); buscaPendente = 0;
    if (r == TECLADO_PRONTO) { snprintf(busca, sizeof busca, "%s", teclado_texto()); linha = 0; filtrar(); }
  }
  if (aba && desejado && capacidadePronta && !rodando && agora - categoriaDesde >= 350u)
    iniciar(modoEpisodios ? V_EPISODIOS : V_LISTA);
  int ini = streammax_tv_primeira(linha, SM_TV_VISIBLE_ROWS, nVisiveis);
  animLinha = anim_mola(animLinha, (float)(linha-ini), dt, 18);
  int ci = streammax_tv_primeira(categoria, 8, totalCategorias());
  animCategoria = anim_mola(animCategoria, (float)(categoria-ci), dt, 18);
  transicao = anim_mola(transicao, 1, dt, 14);
}

static void texto(const char *s, float x, float y, float largura, int grande, float a) {
  TxtLinha t = txt_linha_corta(grande ? TXT_G30B : TXT_PG_ROTULO, s, 238, 241, 255, 255, largura);
  txt_desenhar_alpha(t, x, y, a);
}
static void painel(GfxRect r, float a) { gfx_cor(r, 0.015f, .025f, .025f, .11f, a); }
static void foco(GfxRect r, int ativo, float a) {
  gfx_cor(r, .02f, ativo ? .96f : .16f, ativo ? .025f : .055f, ativo ? .34f : .24f, a);
}
static void abaClique(int i, int b) { (void)b; escolherAba(i); focoAbas = 1; }
void iptvvod_abas_desenhar(float a) {
  const char *nomes[] = {"Ao vivo", "Filmes", "Séries"}; float x = NV_MARGEM_X + 358;
  for (int i = 0; i < 3; i++) {
    if (i && (!xtream_configurado() || (capacidadePronta && disponivel[i-1] == 0))) continue;
    GfxRect r = {x, 36, 150, 43};
    if (aba == i || (focoAbas && abaFoco == i)) foco(r, focoAbas && abaFoco == i, a);
    texto(nomes[i], x + 17, 40, 122, 0, a);
    if (aba == i) gfx_cor((GfxRect){x+15,81,120,3}, 0, .98f,.04f,.36f,a);
    if (a > .95f) ponteiro_alvo(r.x,r.y,r.w,r.h,NULL,abaClique,i,0);
    x += 165;
  }
}
static void categoriaHover(int i, int b) { (void)b; coluna = 0; selecionarCategoria(i); }
static void categoriaClique(int i, int b) { categoriaHover(i,b); coluna = 1; }
static void itemHover(int i, int b) { (void)b; coluna = 1; linha = i; }
static void itemClique(int i, int b) { itemHover(i,b); confirmarItem(); }
static void menuClique(int i, int b) {
  (void)b;
  if (i == 0) buscar();
  else if (i == 1) { soFavoritos = !soFavoritos; linha = 0; filtrar(); }
  else { geracao++; desejado = 1; categoriaDesde = 0; erroLista = 0; }
}
static void assistirClique(int i, int b) { (void)i; (void)b; confirmarItem(); }
static void favClique(int i, int b) { (void)i; (void)b; favAlternar(); }
void iptvvod_desenhar(float a) {
  float x = NV_MARGEM_X, y = SM_TV_BODY_Y, cw = SM_TV_CAT_W, gap = SM_TV_COL_GAP;
  float lx = x + cw + gap, lw = SM_TV_CANAL_W, rx = lx + lw + gap;
  float rw = NV_TELA_W - NV_MARGEM_X - rx, h = NV_TELA_H-y-82;
  gfx_cor((GfxRect){0,0,NV_TELA_W,NV_TELA_H},0,.015f,.02f,.09f,a);
  gfx_cor((GfxRect){0,0,NV_TELA_W,119},0,.015f,.04f,.18f,a);
  gfx_cor((GfxRect){0,116,NV_TELA_W,3},0,.02f,.40f,.92f,a*.65f);
  logoapp_streammax_wordmark((GfxRect){x+18,36,246,44},a);
  iptvvod_abas_desenhar(a);
  char aux[200]; time_t agora = time(NULL); struct tm *tm = localtime(&agora);
  snprintf(aux,sizeof aux,"%02d:%02d",tm ? tm->tm_hour : 0,tm ? tm->tm_min : 0);
  texto(aux,NV_TELA_W-NV_MARGEM_X-100,40,100,1,a);
  painel((GfxRect){x,y,cw,h},a); painel((GfxRect){lx,y,lw,h},a); painel((GfxRect){rx,y,rw,h},a);
  const char *menus[] = {"Buscar",soFavoritos ? "Todos os títulos" : "Favoritos", "Atualizar"};
  const char *icones[] = {"aj_search","pl_star-f","aj_rotate-cw"};
  for (int i = 0; i < 3; i++) {
    GfxRect r={x+2,y+12+i*61,cw-4,57};
    if (coluna == -1 && menuLinha == i && !focoAbas) foco(r,1,a);
    gfx_icone((GfxRect){x+18,r.y+13,26,26},icones[i],.9f,.92f,1,a);
    texto(menus[i],x+57,r.y+12,cw-68,0,a);
    ponteiro_alvo(r.x,r.y,r.w,r.h,NULL,menuClique,i,0);
  }
  texto(modoEpisodios ? "Temporadas" : "Categorias",x+20,y+211,cw-40,0,a*.65f);
  int nc = totalCategorias(), ci = streammax_tv_primeira(categoria,8,nc);
  if (nc) foco((GfxRect){x+2,y+255+animCategoria*72,cw-4,65},coluna==0&&!focoAbas,a);
  for (int i = ci; i < nc && i < ci+8; i++) {
    GfxRect r={x+2,y+255+(i-ci)*72,cw-4,65};
    if (modoEpisodios) snprintf(aux,sizeof aux,temporadas[i] ? "Temporada %d" : "Especiais",temporadas[i]);
    else snprintf(aux,sizeof aux,"%s",cats[aba-1][i].nome);
    texto(aux,x+20,r.y+17,cw-35,0,a);
    ponteiro_alvo(r.x,r.y,r.w,r.h,categoriaHover,categoriaClique,i,0);
  }
  texto(modoEpisodios ? serieAtual.nome : aba == XV_FILMES ? "Filmes" : "Séries",lx+20,y+12,lw-40,0,a);
  int ini = streammax_tv_primeira(linha,SM_TV_VISIBLE_ROWS,nVisiveis);
  if (nVisiveis && coluna >= 1 && !focoAbas) foco((GfxRect){lx+2,y+50+animLinha*72,lw-4,68},1,a);
  for (int j=0; j<SM_TV_VISIBLE_ROWS && ini+j<nVisiveis; j++) {
    int idx = ini+j; XtreamVodItem *it=&itens[indices[idx]];
    GfxRect r={lx+2,y+50+j*72,lw-4,68};
    GLuint tex=it->capa[0]?tex_obter_larg(it->capa,42):0;
    if(tex) gfx_rect((GfxRect){r.x+14,r.y+4,40,59},tex,GFX_CARD,0,0,0,.04f,1,1,1,a*transicao);
    else gfx_icone((GfxRect){r.x+18,r.y+18,30,30},"pl_play",.7f,.77f,1,a);
    texto(it->nome,r.x+70,r.y+7,lw-103,0,a*transicao);
    if(modoEpisodios) snprintf(aux,sizeof aux,"T%d · Episódio %d",it->temporada,it->episodio);
    else snprintf(aux,sizeof aux,"%s%s%s",it->ano,it->ano[0]?" · ":"",aba==XV_FILMES?"Filme sob demanda":"Ver episódios");
    TxtLinha t=txt_linha_corta(TXT_DET_META2,aux,156,176,217,255,lw-100);
    txt_desenhar_alpha(t,r.x+70,r.y+39,a*transicao);
    gfx_cor((GfxRect){r.x+12,r.y+r.h-1,r.w-24,1},0,.3f,.4f,.65f,a*.3f);
    ponteiro_alvo(r.x,r.y,r.w,r.h,itemHover,itemClique,idx,0);
  }
  if (!nVisiveis) {
    const char *msg = rodando || desejado ? "Carregando catálogo…" :
      erroLista == XV_AUTH ? "Acesso recusado pelo provedor. Confira sua conta Xtream." :
      erroLista ? "Não foi possível carregar. Selecione Atualizar para tentar novamente." :
      busca[0] || soFavoritos ? "Nenhum título encontrado nesta categoria." : "O provedor não disponibilizou títulos nesta categoria.";
    txt_bloco_corta(TXT_PG_ROTULO,msg,191,204,233,lx+24,y+120,lw-48,32.0f,a,6);
  }
  XtreamVodItem *it=selecionado();
  if (it) {
    GfxRect capa={rx+24,y+25,185,278};
    GLuint tex=it->capa[0]?tex_obter_larg(it->capa,capa.w):0;
    if(tex) gfx_rect(capa,tex,GFX_CARD,0,0,0,.025f,1,1,1,a*transicao);
    else { painel(capa,a); gfx_icone((GfxRect){capa.x+58,capa.y+102,70,70},"pl_play",.3f,.5f,.9f,a); }
    float tx=capa.x+capa.w+22, tw=rx+rw-24-tx;
    txt_bloco_corta(TXT_G30B,it->nome,244,245,255,tx,y+30,tw,36.0f,a*transicao,5);
    snprintf(aux,sizeof aux,"%s%s%s",it->ano,it->nota[0]?" · Nota ":"",it->nota);
    texto(aux,tx,y+230,tw,0,a*.75f);
    const char *desc=it->sinopse[0]?it->sinopse:modoEpisodios?serieAtual.sinopse:
      "A sinopse não foi informada pelo provedor.";
    txt_bloco_corta(TXT_DET_SIN,desc,190,203,232,rx+25,y+330,rw-50,32.0f,a*transicao,7);
    GfxRect botao={rx+24,y+h-160,rw-48,62};
    foco(botao,coluna==2&&acao==0&&!focoAbas,a);
    texto(aba==XV_SERIES&&!modoEpisodios?"Ver episódios":"Assistir",botao.x+23,botao.y+16,botao.w-46,0,a);
    ponteiro_alvo(botao.x,botao.y,botao.w,botao.h,NULL,assistirClique,0,0);
    botao.y+=76;
    if(coluna==2&&acao==1&&!focoAbas) foco(botao,1,a);
    texto(favorito(it)?"★ Remover dos favoritos":"☆ Adicionar aos favoritos",botao.x+14,botao.y+13,botao.w-28,0,a*.85f);
    ponteiro_alvo(botao.x,botao.y,botao.w,botao.h,NULL,favClique,0,0);
  } else texto("VOD da sua lista IPTV",rx+25,y+40,rw-50,1,a*.8f);
  if (totalItens>nItens && !modoEpisodios) snprintf(aux,sizeof aux,"Mostrando %d de %d. Use as categorias do provedor.",nItens,totalItens);
  else snprintf(aux,sizeof aux,"%d título(s)%s%s  ·  Setas: navegar  ·  OK: abrir  ·  Voltar",nVisiveis,busca[0]?"  ·  Busca: ":"",busca);
  texto(aux,x,NV_TELA_H-49,NV_TELA_W-2*x,0,a*.65f);
  if (buscaPendente && teclado_aberto()) { ponteiro_camada(); teclado_desenhar(SDL_GetTicks()); }
}
int iptvvod_pediu_tocar(CatItem *out, char *url, unsigned n) {
  if (!tocar) return 0;
  tocar=0; XtreamVodItem *it=selecionado();
  if (!it || !out || !xtream_vod_url(aba,it,url,n,sessao,revisao)) return 0;
  memset(out,0,sizeof *out);
  if (!xtream_vod_id_video(modoEpisodios ? XV_EPISODIO : aba,it,out->imdb,sizeof out->imdb)) return 0;
  // Cada episodio tem identidade propria: o ponto salvo nunca abre outro.
  snprintf(out->tipo,sizeof out->tipo,"movie");
  if (modoEpisodios) snprintf(out->titulo,sizeof out->titulo,"%.100s · T%d E%d",serieAtual.nome,it->temporada,it->episodio);
  else snprintf(out->titulo,sizeof out->titulo,"%s",it->nome);
  snprintf(out->poster,sizeof out->poster,"%s",it->capa);
  snprintf(out->sinopse,sizeof out->sinopse,"%s",it->sinopse);
  snprintf(out->genero,sizeof out->genero,"%s",modoEpisodios?"Série · VOD IPTV":"Filme · VOD IPTV");
  snprintf(out->meta,sizeof out->meta,"%.23s",it->ano);
  return 1;
}
