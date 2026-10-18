#include "login.h"
#include "iconeapp.h"
#include "logoapp.h"
#include "sessao.h"
#include "nuvem.h"
#include "gfx.h"
#include "text.h"
#include "anim.h"
#include "layout.h"
#include "teclado.h"
#include "ponteiro.h"
#include "ajustes.h"
#include "idioma.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// StreamMax: autenticacao direta por e-mail e senha, sem fluxo QR na TV.
#define LG_BLOCO_W      lgBlocoW()
#define LG_PILL_H         76.0f
#define LG_EMAIL_PILL_W  560.0f
#define LG_CAMPO_W       760.0f
#define LG_CAMPO_H        88.0f
#define LG_MOSTRAR_W     150.0f
static float lgBlocoW(void) {
#ifdef NV_TOUCH_UI
  if (NV_TELA_H > NV_TELA_W) return NV_TELA_W - 2.0f * NV_MARGEM_X;
#endif
  return 1100.0f;
}

// Login por e-mail/senha com teclado adaptado para controle remoto.
enum { LE_EMAIL = 0, LE_SENHA, LE_MOSTRAR, LE_ENTRAR, LE_N };
static int foco;
static int campoAberto = -1;
static char email[TECLADO_LONGO + 1], senha[TECLADO_LONGO + 1];
static char aviso[160];
static float animFoco[LE_N];
static int esperando;
static int senhaVisivel;

// O e-mail: minusculas (o servidor compara sem caixa), digitos e o que um
// endereco usa. A senha: todo ASCII imprimivel, com espaco — e a senha que a
// pessoa criou no site, nao uma que o app escolhe.
static const char *ALFA_EMAIL = "abcdefghijklmnopqrstuvwxyz0123456789@._-+";
static const char *ALFA_SENHA =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
  " .,_-@!#$%&*+=?/\\:;'\"()[]{}<>^~`|";

static void apagarTexto(char *p, size_t n) { volatile char *v = p; while (n--) *v++ = 0; }

static void abrirCampo(int c) {
  campoAberto = c;
  if (c == LE_EMAIL) {
    teclado_abrir_com("E-mail", "O e-mail da sua conta StreamMax.", 120, ALFA_EMAIL, email);
    teclado_tipo(TECLADO_TIPO_EMAIL);
  } else {
    // A senha VOLTA para a modal (em pontos): errar uma letra no fim de uma
    // senha longa nao pode obrigar a redigitar tudo no D-pad.
    teclado_abrir_com("Senha", "A senha da sua conta StreamMax. Ela não aparece na tela.", 128, ALFA_SENHA, senha);
    teclado_tipo(TECLADO_TIPO_SENHA);
    teclado_mascarar(!senhaVisivel);
  }
}
static void enviar(void) {
  if (sessao_estado() == SES_EMAIL) return;
  if (!email[0] || !senha[0]) {
    snprintf(aviso, sizeof aviso, "%s", "Preencha o e-mail e a senha.");
    foco = email[0] ? LE_SENHA : LE_EMAIL;
    return;
  }
  aviso[0] = 0;
  sessao_login_email(email, senha);
  esperando = sessao_estado() == SES_EMAIL;
  // A senha sai da memoria da tela ja: errada, a pessoa digita de novo.
  apagarTexto(senha, sizeof senha);
}
static void okEmail(void) {
  switch (foco) {
    case LE_EMAIL: case LE_SENHA: abrirCampo(foco); break;
    case LE_MOSTRAR: senhaVisivel = !senhaVisivel; break;
    case LE_ENTRAR: enviar(); break;
    default: break;
  }
}

// Ponteiro e dedo: posiciona o foco no campo ou botao.
static void ptFocoEmail(int i, int b) { (void)b; foco = i; }

// FUNDO (1.7.2): a arte das listras, a mesma familia do splash. A abertura
// dissolve o logo por cima dela (abertura_fundo_fica), entao a tela de login
// nasce do splash sem corte. 1280x720 ampliada: e fundo, nao precisa de mais.
// Carregada no primeiro desenho e solta ao entrar (login_soltar).
static char   fundoCam[600];
static GLuint texFundo;
static int    tentouFundo;

void login_recursos(const char *dirArte) {
  snprintf(fundoCam, sizeof fundoCam, "%s/marcas/login-fundo.jpg", dirArte ? dirArte : ".");
}

static void carregarFundo(void) {
  SDL_Surface *s, *c;
  tentouFundo = 1;
  if (!fundoCam[0]) return;
  s = IMG_Load(fundoCam);
  if (!s) { printf("[login] sem o fundo (%s)\n", fundoCam); return; }
  c = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_ABGR8888, 0);   // RGBA em bytes
  SDL_FreeSurface(s);
  if (!c) return;
  glGenTextures(1, &texFundo);
  glBindTexture(GL_TEXTURE_2D, texFundo);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, c->w, c->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, c->pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  gfx_tex_esquecer(0);   // o gfx guarda a ultima textura ligada
  SDL_FreeSurface(c);
}

int login_fundo_desenhar(float alfa) {
  if (!tentouFundo) carregarFundo();
  if (!texFundo) return 0;
  gfx_tex_aspect_atual = 0.0f;
  gfx_rect((GfxRect){ 0, 0, NV_TELA_W, NV_TELA_H }, texFundo, GFX_TEXTO,
           0, 0, 0, 0.0f, 1, 1, 1, alfa);
  return 1;
}

void login_soltar(void) {
  if (texFundo) { gfx_tex_esquecer(texFundo); glDeleteTextures(1, &texFundo); texFundo = 0; }
  tentouFundo = 0;
}   // conteudo ja desenhado, para nao refazer por quadro

void login_iniciar(void) {
  foco = LE_EMAIL;
  campoAberto = -1;
  aviso[0] = 0;
  esperando = 0;
  senhaVisivel = 0;
  apagarTexto(senha, sizeof senha);
  // O login por e-mail nao necessita de sessao anonima ou codigo QR.
  // O usuario inicia a requisicao apenas ao escolher Entrar.
}

void login_evento(const SDL_Event *e) {
  SDL_Keycode k;
  int ok, voltar;
  if (teclado_aberto()) { teclado_evento(e); return; }
  if (e->type != SDL_KEYDOWN) return;
  k = e->key.keysym.sym;
  ok = (k == SDLK_RETURN || k == SDLK_KP_ENTER || k == SDLK_SPACE) && !e->key.repeat;
  voltar = k == SDLK_AC_BACK || k == SDLK_ESCAPE || e->key.keysym.scancode == NV_SCANCODE_BACK;
  if (voltar || sessao_estado() == SES_EMAIL) return;
  if (k == SDLK_UP)
    foco = foco == LE_ENTRAR ? LE_SENHA : LE_EMAIL;
  else if (k == SDLK_DOWN)
    foco = foco == LE_EMAIL ? LE_SENHA : LE_ENTRAR;
  else if (k == SDLK_RIGHT && foco == LE_SENHA) foco = LE_MOSTRAR;
  else if (k == SDLK_LEFT && foco == LE_MOSTRAR) foco = LE_SENHA;
  else if (ok) okEmail();
}

void login_atualizar(float dt, Uint32 agora) {
  int i, r;
  sessao_passo((unsigned)agora);
  teclado_atualizar(dt, agora);
  r = teclado_resultado();
  if (r != TECLADO_NADA && campoAberto >= 0) {
    int seguir = 0;
    if (campoAberto == LE_SENHA) senhaVisivel = !teclado_mascarado();
    if (r == TECLADO_PRONTO) {
      // PRONTO/ENTER AVANCA (#216): do e-mail direto para a senha, da senha
      // para Entrar. So com o controle, sao dois OK a menos.
      if (campoAberto == LE_EMAIL) {
        snprintf(email, sizeof email, "%s", teclado_texto());
        foco = LE_SENHA;
        seguir = 1;
      } else {
        snprintf(senha, sizeof senha, "%s", teclado_texto());
        foco = LE_ENTRAR;
      }
      aviso[0] = 0;
    }
    teclado_esquecer();
    campoAberto = -1;
    if (seguir) abrirCampo(LE_SENHA);
  }
  // Falhou: o foco vai para onde a pessoa conserta.
  // (O estado muda no fio do pedido, entre quadros: por isso a bandeira.)
  if (esperando && sessao_estado() != SES_EMAIL) {
    esperando = 0;
    if (!sessao_logada()) foco = LE_SENHA;
  }
  for (i = 0; i < LE_N; i++)
    animFoco[i] = anim_mola(animFoco[i], foco == i && !teclado_aberto() ? 1.0f : 0.0f,
                            dt, NV_MOLA_FOCO);
}

// Paleta fixa do StreamMax no Android TV, consistente com o player.
// Sem assets ou shaders adicionais: apenas cores nas primitivas existentes.
static void loginAcento(float *r, float *g, float *b) {
#ifdef NV_ANDROID
  *r = 0.14f; *g = 0.59f; *b = 0.96f;
#else
  ajustes_acento(r, g, b);
#endif
}

// Cartao central somente na TV Android (canvas 1920x1080 da TCL P7K).
// No celular e nas demais plataformas o layout tradicional permanece.
static void painelLoginStreamMax(void) {
#if defined(NV_ANDROID) && !defined(NV_TOUCH_UI)
  GfxRect painel = { (NV_TELA_W - 1050.0f) * 0.5f, 82.0f, 1050.0f, 768.0f };
  // Fundo azul-marinho sem blur, translucido o suficiente para o splash.
  gfx_cor(painel, 0.033f, 0.025f, 0.041f, 0.076f, 0.94f);
  // Assinatura azul discreta, sem animacao permanente.
  gfx_cor((GfxRect){ painel.x + 100.0f, painel.y + 14.0f,
                      120.0f, 4.0f }, 0.5f, 0.14f, 0.59f, 0.96f, 0.90f);
#endif
}

// Pilula de botao: cheia e clara no foco, vidro discreto fora dele.
static void pilula(float cx, float y, float w, const char *rot, float k, float alpha,
                   PonteiroFn focar, int i) {
  GfxRect pill = { cx - w * 0.5f, y, w, LG_PILL_H };
  TxtLinha t;
  int tom = k >= 0.5f ? 18 : 244;
#ifdef NV_ANDROID
  // Botao primario azul, sem a pilula branca do Legacy.
  gfx_cor(pill, NV_RAIO_PILL, anim_mistura(0.07f, 0.14f, k),
          anim_mistura(0.23f, 0.59f, k), anim_mistura(0.46f, 0.96f, k), alpha);
#else
  gfx_cor(pill, NV_RAIO_PILL, 1.0f, 1.0f, 1.0f, anim_mistura(0.12f, 0.92f, k) * alpha);
#endif
  t = txt_linha_corta(TXT_BODY, rot, tom, tom, tom + 2 > 255 ? 255 : tom + 2, 255, w - 48.0f);
  txt_desenhar_alpha(t, cx - t.w * 0.5f, y + (LG_PILL_H - t.h) * 0.5f, alpha);
  if (focar) ponteiro_alvo(pill.x, pill.y, pill.w, pill.h, focar, NULL, i, 0);
}

static void linhaCentrada(TxtEstilo est, const char *s, int r, int g, int b,
                          float y, float alpha) {
  TxtLinha l = txt_linha_corta(est, s, r, g, b, 255, LG_BLOCO_W);
  txt_desenhar_alpha(l, (NV_TELA_W - l.w) * 0.5f, y, alpha);
}
static float tituloLogin(float y) {
#ifdef NV_TOUCH_UI
  const char *s = i18n("Entrar na sua conta");
  if (NV_TELA_H > NV_TELA_W && txt_largura(TXT_TITULO1, s) > LG_BLOCO_W)
    return txt_bloco_corta(TXT_TITULO1, s, 255, 255, 255,
                          (NV_TELA_W - LG_BLOCO_W) * 0.5f, y, LG_BLOCO_W,
                          86.0f, 1.0f, 2) + 42.0f;
#endif
#ifdef NV_ANDROID
  linhaCentrada(TXT_TITULO1, "Bem-vindo ao StreamMax", 242, 247, 255, y, 1.0f);
#else
  linhaCentrada(TXT_TITULO1, "Entrar na sua conta", 255, 255, 255, y, 1.0f);
#endif
  return 118.0f;
}

// Campo da tela de e-mail: rotulo em cima, caixa com o valor (ou pontos).
static void campo(float y, int i, const char *rotulo, const char *valor, int oculto) {
  float x = (NV_TELA_W - LG_CAMPO_W) * 0.5f, k = animFoco[i];
  GfxRect cx = { x, y + 42.0f, LG_CAMPO_W, LG_CAMPO_H };
  float ar, ag, ab;
#ifdef NV_ANDROID
  TxtLinha r = txt_linha(TXT_CAPTION, rotulo, 192, 208, 234, 255);
#else
  TxtLinha r = txt_linha(TXT_CAPTION, rotulo, 176, 178, 186, 255);
#endif
  txt_desenhar_alpha(r, x + 4.0f, y, 1.0f);
  loginAcento(&ar, &ag, &ab);
#ifdef NV_ANDROID
  gfx_cor(cx, NV_RAIO_CARD, anim_mistura(0.065f, 0.095f, k),
          anim_mistura(0.091f, 0.145f, k),
          anim_mistura(0.150f, 0.225f, k), 0.99f);
#else
  gfx_cor(cx, NV_RAIO_CARD, 1.0f, 1.0f, 1.0f, anim_mistura(0.07f, 0.13f, k));
#endif
  if (k > 0.01f)
    gfx_vidro_aro((GfxRect){ cx.x - 6, cx.y - 6, cx.w + 12, cx.h + 12 }, NV_RAIO_CARD, 2.5f,
                  ar, ag, ab, 0.95f * k);
  ponteiro_alvo(cx.x, y, cx.w, cx.h + 42.0f, ptFocoEmail, NULL, i, 0);
  if (valor[0]) {
    char pontos[TECLADO_LONGO * 3 + 1];
    TxtLinha t;
    float larg = cx.w - 48.0f - (i == LE_SENHA ? LG_MOSTRAR_W + 12.0f : 0.0f);
    if (oculto) {
      size_t j, n = strlen(valor);
      for (j = 0; j < n && j < TECLADO_LONGO; j++) memcpy(pontos + j * 3, "\xE2\x80\xA2", 3);
      pontos[j * 3] = 0;
    }
    // O valor e da pessoa; txt_linha procura traducao, e um e-mail nunca casa.
    t = txt_linha(TXT_HEADLINE, oculto ? pontos : valor, 240, 242, 248, 255);
    gfx_recorte(cx.x + 24.0f, cx.y, larg, cx.h);
    txt_desenhar_alpha(t, cx.x + 24.0f - (t.w > larg ? t.w - larg : 0.0f),
                       cx.y + (cx.h - t.h) * 0.5f, 1.0f);
    gfx_sem_recorte();
  } else {
    // Campo vazio continua identificavel mesmo visto a distancia da TV.
    TxtLinha t = txt_linha(TXT_BODY,
        i == LE_EMAIL ? "Digite seu e-mail" : "Digite sua senha",
        132, 155, 191, 255);
    txt_desenhar_alpha(t, cx.x + 24.0f,
                      cx.y + (cx.h - t.h) * 0.5f, 1.0f);
  }
}

static void desenharEmail(float y, Uint32 agora) {
  SesEstado st = sessao_estado();
  const char *falha = sessao_erro_email();
  (void)agora;
#ifdef NV_ANDROID
  linhaCentrada(TXT_BODY, "Use seu e-mail e senha para continuar.", 179, 196, 222, y, 1.0f);
#else
  linhaCentrada(TXT_BODY, "Use o e-mail e a senha da sua conta StreamMax.", 176, 178, 186, y, 1.0f);
#endif
  y += 84.0f;
  campo(y, LE_EMAIL, "E-mail", email, 0);
  y += 42.0f + LG_CAMPO_H + 30.0f;
  campo(y, LE_SENHA, "Senha", senha, !senhaVisivel);
  // MOSTRAR/OCULTAR dentro do campo, a direita (como nos sites).
  { float x = (NV_TELA_W + LG_CAMPO_W) * 0.5f - LG_MOSTRAR_W - 12.0f;
    float yb = y + 42.0f + (LG_CAMPO_H - 60.0f) * 0.5f, k = animFoco[LE_MOSTRAR];
    GfxRect b = { x, yb, LG_MOSTRAR_W, 60.0f };
    int tom = k >= 0.5f ? 18 : 230;
    TxtLinha t = txt_linha(TXT_CAPTION, senhaVisivel ? "Ocultar" : "Mostrar", tom, tom, tom, 255);
#ifdef NV_ANDROID
    gfx_cor(b, NV_RAIO_PILL, anim_mistura(0.09f, 0.16f, k),
            anim_mistura(0.17f, 0.59f, k), anim_mistura(0.27f, 0.96f, k), 1.0f);
#else
    gfx_cor(b, NV_RAIO_PILL, 1.0f, 1.0f, 1.0f, anim_mistura(0.10f, 0.92f, k));
#endif
    txt_desenhar_alpha(t, b.x + (b.w - t.w) * 0.5f, b.y + (b.h - t.h) * 0.5f, 1.0f);
    ponteiro_alvo(b.x, b.y, b.w, b.h, ptFocoEmail, NULL, LE_MOSTRAR, 0); }
  y += 42.0f + LG_CAMPO_H + 48.0f;
  pilula(NV_TELA_W * 0.5f, y, LG_EMAIL_PILL_W, st == SES_EMAIL ? "Entrando…" : "Entrar",
         animFoco[LE_ENTRAR], 1.0f, ptFocoEmail, LE_ENTRAR);
  y += LG_PILL_H + 36.0f;
  if (aviso[0]) linhaCentrada(TXT_BODY, aviso, 236, 108, 108, y, 1.0f);
  else if (st != SES_EMAIL && falha[0]) {
    // Ja vem traduzida de sessao.c (formato com i18n): nao passa de novo.
    TxtLinha l = txt_linha_corta(TXT_BODY, falha, 236, 108, 108, 255, LG_BLOCO_W);
    txt_desenhar_alpha(l, (NV_TELA_W - l.w) * 0.5f, y, 1.0f);
  }
}

void login_desenhar(Uint32 agora) {
  float y;
  (void)agora;
  { GfxRect tela = { 0, 0, NV_TELA_W, NV_TELA_H };
    if (!login_fundo_desenhar(1.0f))
      gfx_cor(tela, 0.0f, NV_COR_FUNDO_R, NV_COR_FUNDO_G, NV_COR_FUNDO_B, 1.0f);
#if defined(NV_ANDROID) && !defined(NV_TOUCH_UI)
    // As listras do splash viram textura sutil, nao dominam o formulario.
    gfx_cor(tela, 0.0f, 0.012f, 0.022f, 0.045f, 0.80f);
    painelLoginStreamMax();
#endif
  }
  ponteiro_camada();
  ponteiro_alvo(0, 0, NV_TELA_W, NV_TELA_H, NULL, NULL, 0, 0);
#if defined(NV_ANDROID) && !defined(NV_TOUCH_UI)
  // Marca tipografica propria, sem reintroduzir o simbolo do Nuvio Legacy.
  { TxtLinha marca = txt_linha(TXT_G30B, "STREAMMAX", 218, 234, 255, 255);
    txt_desenhar_alpha(marca, NV_MARGEM_X, NV_MARGEM_Y + 20.0f, 1.0f); }
#else
  logoapp_marca((GfxRect){ NV_MARGEM_X, NV_MARGEM_Y, 72.0f, 72.0f }, 1.0f);
#endif
  y = 118.0f;
  y += tituloLogin(y);

  if (!nuvem_pronta()) {
    linhaCentrada(TXT_HEADLINE, "Login indisponível neste pacote.",
                  236, 108, 108, y, 1.0f);
    linhaCentrada(TXT_BODY,
                  "A compilação precisa da URL e da chave pública do servidor.",
                  176, 178, 186, y + 62.0f, 1.0f);
    return;
  }
  desenharEmail(y, agora);
  teclado_desenhar(agora);
}

int login_concluido(void) { return sessao_logada(); }
