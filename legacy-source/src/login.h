// Acesso StreamMax por e-mail/senha e criacao de conta na propria TV.
// Controle remoto, teclado e ponteiro usam o mesmo formulario. Sem QR.
#ifndef NV_LOGIN_H
#define NV_LOGIN_H
#include <SDL2/SDL.h>

// Pasta da arte (o fundo da tela vem de art/marcas/login-fundo.jpg).
void login_recursos(const char *dirArte);
// Desenha o fundo das listras na tela inteira (carrega no primeiro uso).
// 0 quando o arquivo nao existe; quem chama desenha o proprio fundo.
// Usado tambem pela escolha de perfil com o mural desligado.
int  login_fundo_desenhar(float alfa);
// Solta a textura do fundo; chamado quando a pessoa passa do login e da
// escolha de perfil.
void login_soltar(void);
void login_iniciar(void);
void login_evento(const SDL_Event *e);
void login_atualizar(float dt, Uint32 agora);
void login_desenhar(Uint32 agora);

// 1 quando a tela terminou o trabalho dela (ha sessao) e o app deve seguir.
int  login_concluido(void);

#endif
