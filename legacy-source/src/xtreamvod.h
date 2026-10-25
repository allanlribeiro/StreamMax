// VOD fornecido pelo cadastro Xtream do perfil. Sem URLs privadas no catalogo.
#ifndef STREAMMAX_XTREAM_VOD_H
#define STREAMMAX_XTREAM_VOD_H
#include <stdint.h>
#include "perfisessao.h"
enum { XV_FILMES = 1, XV_SERIES = 2, XV_EPISODIO = 3 };
enum { XV_REDE = -1, XV_AUTH = -2, XV_RESPOSTA = -3, XV_CANCELADO = -4 };
#define XV_MAX_CATEGORIAS 256
#define XV_MAX_ITENS 4000
typedef struct { char id[32], nome[96]; } XtreamVodCategoria;
typedef struct {
  char id[32], nome[160], categoria[32], capa[512], extensao[16];
  char sinopse[900], ano[24], nota[24];
  int temporada, episodio;
} XtreamVodItem;
// A revisao muda tambem quando se edita o cadastro sem trocar de perfil.
uint64_t xtream_revisao(void);
int xtream_vod_categorias(int tipo, XtreamVodCategoria *out, int max,
                          NvPerfilSessao sessao, uint64_t revisao);
// categoria vazia consulta o catalogo sem filtro. Retorna total pelo ponteiro.
int xtream_vod_itens(int tipo, const char *categoria, XtreamVodItem *out, int max,
                    int *total, NvPerfilSessao sessao, uint64_t revisao);
int xtream_vod_episodios(const char *serie, XtreamVodItem *out, int max,
                        int *total, NvPerfilSessao sessao, uint64_t revisao);
// Monta na hora de assistir, com a mesma revisao que publicou os metadados.
int xtream_vod_url(int tipo, const XtreamVodItem *item, char *url, unsigned n,
                   NvPerfilSessao sessao, uint64_t revisao);
int xtream_vod_identidade(int tipo, const char *id, char *out, unsigned n);
// IDs persistidos so contem conta opaca, tipo, id numerico e extensao.
// A URL com a credencial e reconstruida para o perfil atual ao retomar.
int xtream_vod_id_video(int tipo, const XtreamVodItem *item, char *out, unsigned n);
int xtream_vod_resolver(const char *id, char *url, unsigned n);
#endif
