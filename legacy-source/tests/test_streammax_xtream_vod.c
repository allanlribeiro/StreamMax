#define _POSIX_C_SOURCE 200809L
#include "xtream.h"
#include "xtreamvod.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *resposta = "[]";
static int http = 200, perfil = 1, chamadas, trocarCadastro, trocarPerfil;
static uint64_t abertura = 1;
static char pedido[1400];
uint64_t perfis_sessao_geracao(void) { return abertura; }
int perfis_sessao_atual(uint64_t s) { return s == abertura; }
int perfis_sessao_travar(uint64_t s) { return s == abertura; }
void perfis_sessao_destravar(void) {}
int perfis_ativo(void) { return perfil; }
char *dados_ler(const char *nome) { (void)nome; return NULL; }
int dados_gravar(const char *n, const char *s) { (void)n; (void)s; return 1; }
int dados_apagar(const char *nome) { (void)nome; return 1; }
int ajustes_livetv_formato(void) { return 0; }
char *rede_baixar_st_retry(const char *url, int prazo, const char *const *cab, int *st, int *retry) {
  (void)prazo; (void)cab; *st = http; *retry = 0; chamadas++;
  snprintf(pedido, sizeof pedido, "%s", url);
  if (trocarCadastro) { trocarCadastro = 0; xtream_definir_usuario("outra-conta"); }
  if (trocarPerfil) { trocarPerfil = 0; perfil++; abertura++; }
  return resposta ? strdup(resposta) : NULL;
}
static void credenciais(void) {
  xtream_definir_servidor("https://iptv.example.invalid:443/");
  xtream_definir_usuario("teste usuario");
  xtream_definir_senha("teste/senha");
}
int main(void) {
  XtreamVodCategoria cats[4]; XtreamVodItem itens[5]; char url[1400], id[64], outro[64]; int total;
  credenciais(); uint64_t rev = xtream_revisao(), s = abertura;
  resposta = "[{\"category_id\":12,\"category_name\":\"Ação\"},{\"category_id\":\"13\",\"category_name\":\"Séries\"}]";
  assert(xtream_vod_categorias(XV_FILMES,cats,4,s,rev)==2);
  assert(!strcmp(cats[0].id,"12") && !strcmp(cats[0].nome,"Ação"));
  assert(strstr(pedido,"action=get_vod_categories"));
  assert(xtream_vod_categorias(XV_SERIES,cats,4,s,rev)==2);
  assert(strstr(pedido,"action=get_series_categories"));

  resposta = "[{\"stream_id\":51,\"name\":\"Filme de teste\",\"stream_icon\":\"https://example.invalid/capa.jpg\",\"category_id\":12,\"container_extension\":\"mkv\",\"year\":2026,\"rating\":8.5},{\"stream_id\":52,\"name\":\"Outro\",\"category_id\":13}]";
  assert(xtream_vod_itens(XV_FILMES,"12",itens,5,&total,s,rev)==1 && total==1);
  assert(strstr(pedido,"get_vod_streams&category_id=12"));
  assert(!strcmp(itens[0].ano,"2026") && !strcmp(itens[0].nota,"8.5"));
  assert(xtream_vod_url(XV_FILMES,&itens[0],url,sizeof url,s,rev));
  assert(!strcmp(url,"https://iptv.example.invalid:443/movie/teste%20usuario/teste%2Fsenha/51.mkv"));
  assert(!xtream_vod_url(XV_FILMES,&itens[0],url,10,s,rev) && url[0]==0);
  strcpy(itens[0].extensao,"../../bad");
  assert(!xtream_vod_url(XV_FILMES,&itens[0],url,sizeof url,s,rev));
  int antes = chamadas;
  assert(xtream_vod_itens(XV_FILMES,"12&password=bad",itens,5,&total,s,rev)==XV_RESPOSTA);
  assert(chamadas==antes);
  assert(xtream_vod_itens(XV_FILMES,"",itens,1,&total,s,rev)==1 && total==2);

  resposta = "[{\"info\":{\"name\":\"NAO USAR\",\"series_id\":999},\"series_id\":71,\"name\":\"Série correta\",\"cover\":\"https://example.invalid/serie.jpg\",\"plot\":\"Sinopse\"}]";
  assert(xtream_vod_itens(XV_SERIES,"",itens,5,&total,s,rev)==1);
  assert(!strcmp(itens[0].nome,"Série correta") && !strcmp(itens[0].id,"71"));

  resposta = "{\"episodes\":{\"2\":[{\"id\":902,\"episode_num\":2,\"title\":\"Segundo\",\"container_extension\":\"mp4\",\"info\":{\"id\":9999,\"plot\":\"Sinopse do episódio\"}}],\"1\":[{\"id\":801,\"season\":1,\"episode_num\":1,\"title\":\"Primeiro\",\"container_extension\":\"mkv\"},{\"id\":\"bad/91\",\"episode_num\":9}]}}";
  assert(xtream_vod_episodios("71",itens,5,&total,s,rev)==2 && total==2);
  assert(itens[0].temporada==1 && itens[1].temporada==2);
  assert(!strcmp(itens[0].id,"801") && !strcmp(itens[1].id,"902"));
  assert(!strcmp(itens[1].sinopse,"Sinopse do episódio"));
  assert(xtream_vod_url(XV_SERIES,&itens[0],url,sizeof url,s,rev));
  assert(strstr(url,"/series/teste%20usuario/teste%2Fsenha/801.mkv"));
  resposta = "{\"episodes\":[]}";
  assert(xtream_vod_episodios("71",itens,5,&total,s,rev)==0);

  resposta = "[]";
  assert(xtream_vod_categorias(XV_FILMES,cats,4,s,rev)==0);
  resposta = "<html>Erro do provedor</html>";
  assert(xtream_vod_categorias(XV_FILMES,cats,4,s,rev)==XV_RESPOSTA);
  resposta = "{\"user_info\":{\"auth\":0}}";
  assert(xtream_vod_itens(XV_FILMES,"",itens,5,&total,s,rev)==XV_AUTH);
  http = 401;
  assert(xtream_vod_itens(XV_FILMES,"",itens,5,&total,s,rev)==XV_AUTH);
  http = 429;
  assert(xtream_vod_itens(XV_FILMES,"",itens,5,&total,s,rev)==XV_REDE);
  http = 200; resposta = NULL;
  assert(xtream_vod_itens(XV_FILMES,"",itens,5,&total,s,rev)==XV_REDE);
  resposta = "[{\"stream_id\":1,\"name\":\"Teste\"}]";
  trocarCadastro = 1;
  assert(xtream_vod_itens(XV_FILMES,"",itens,5,&total,s,rev)==XV_CANCELADO);
  assert(!xtream_vod_url(XV_FILMES,&itens[0],url,sizeof url,s,rev));
  credenciais(); rev = xtream_revisao();
  assert(xtream_vod_identidade(XV_FILMES,"1",id,sizeof id));
  assert(!strstr(id,"usuario") && !strstr(id,"senha"));
  xtream_definir_usuario("conta diferente");
  assert(xtream_vod_identidade(XV_FILMES,"1",outro,sizeof outro) && strcmp(id,outro));
  rev = xtream_revisao(); trocarPerfil = 1;
  assert(xtream_vod_itens(XV_FILMES,"",itens,5,&total,s,rev)==XV_CANCELADO);
  puts("OK: categorias, filmes, series, episodios, URLs, erros e isolamento de conta/perfil");
}
