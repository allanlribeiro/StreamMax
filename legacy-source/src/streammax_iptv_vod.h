// Secao IPTV isolada do catalogo principal. Canais continuam no guia;
// filmes e series sao consultados somente quando a aba for aberta.
#ifndef STREAMMAX_IPTV_VOD_H
#define STREAMMAX_IPTV_VOD_H
#include <SDL2/SDL.h>
#include <stddef.h>

typedef struct {
  char id[80];        // xtvod:<id> ou xtepisode:<id>
  char nome[192];
  char poster[512];
  char url[2048];     // CONFIDENCIAL: servidor/usuario/senha, somente no player
  int temporada, episodio;
} StreamMaxVodPedido;

void streammax_iptv_sessao(void);
int  streammax_iptv_modo(void);             // 0=ao vivo, 1=filmes, 2=series
void streammax_iptv_mudar(int modo);
int  streammax_iptv_evento(const SDL_Event *e); // so modo VOD
void streammax_iptv_atualizar(void);
void streammax_iptv_desenhar(float alfa);
void streammax_iptv_abas(float alfa, int foco);
int  streammax_iptv_pediu(StreamMaxVodPedido *out);

#endif
