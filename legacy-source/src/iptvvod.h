#ifndef STREAMMAX_IPTV_VOD_H
#define STREAMMAX_IPTV_VOD_H
#include <SDL2/SDL.h>
#include "catalogo.h"
void iptvvod_abrir_sessao(void);
void iptvvod_esquecer_favoritos(void);
void iptvvod_preparar(void);
void iptvvod_atualizar(float dt, Uint32 agora);
int iptvvod_ativo(void);
int iptvvod_tem_abas(void);
void iptvvod_focar_abas(void);
// Volta para Ao vivo sem perder a lista; utilizado ao adotar um canal.
void iptvvod_ao_vivo(void);
int iptvvod_evento(const SDL_Event *e);
void iptvvod_abas_desenhar(float a);
void iptvvod_desenhar(float a);
int iptvvod_parar_preview(void);
int iptvvod_pediu_tocar(CatItem *it, char *url, unsigned n);
#endif
