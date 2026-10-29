#include <assert.h>
#include "../src/login.c"

/* Eventos, validacao e layout reais; teclado, sessao e desenho controlados. */
float nv_layout_w = 1920, nv_layout_h = 1080;
static SesEstado status = SES_DESLOGADO;
static int requestCount, requestSignup, keyboardOpen, keyboardResult, masked;
static char keyboardText[256], keyboardTitle[80], sentPassword[256];
static const char *sessionError = "";
static float bottom;
static int labels[LE_N];
int layout_modo_mobile(void) { return 0; }
SesEstado sessao_estado(void) { return status; }
int sessao_logada(void) { return status == SES_LOGADO; }
int sessao_email_ocupada(void) { return status == SES_EMAIL || status == SES_CADASTRO; }
void sessao_passo(unsigned now) { (void)now; }
const char *sessao_erro_email(void) { return sessionError; }
void sessao_login_email(const char *e, const char *p) {
  assert(e[0] && p[0]); requestCount++; requestSignup = 0;
  snprintf(sentPassword, sizeof sentPassword, "%s", p); status = SES_EMAIL;
}
void sessao_cadastrar_email(const char *e, const char *p) {
  sessao_login_email(e, p); requestSignup = 1; status = SES_CADASTRO;
}
int teclado_aberto(void) { return keyboardOpen; }
void teclado_abrir_com(const char *title, const char *tip, int max, const char *alphabet, const char *initial) {
  (void)tip; (void)alphabet; assert(max > 0);
  snprintf(keyboardTitle, sizeof keyboardTitle, "%s", title);
  snprintf(keyboardText, sizeof keyboardText, "%s", initial); keyboardOpen = 1;
}
void teclado_tipo(int t) { (void)t; }
void teclado_mascarar(int m) { masked = m; }
int teclado_mascarado(void) { return masked; }
void teclado_evento(const SDL_Event *e) { (void)e; }
void teclado_atualizar(float dt, Uint32 now) { (void)dt; (void)now; }
int teclado_resultado(void) { int r = keyboardResult; keyboardResult = 0; return r; }
const char *teclado_texto(void) { return keyboardText; }
void teclado_esquecer(void) { memset(keyboardText, 0, sizeof keyboardText); keyboardOpen = 0; }
static void bounds(float x, float y, float w, float h) {
  assert(x >= -0.1f && y >= 0 && x + w <= NV_TELA_W + 0.1f && y + h <= NV_TELA_H + 0.1f);
  if (y + h > bottom) bottom = y + h;
}
void gfx_cor(GfxRect r, float radius, float cr, float cg, float cb, float a) {
  (void)radius; (void)cr; (void)cg; (void)cb; (void)a; bounds(r.x,r.y,r.w,r.h);
}
void gfx_vidro_aro(GfxRect r, float radius, float e, float cr, float cg, float cb, float a) {
  (void)radius; (void)e; (void)cr; (void)cg; (void)cb; (void)a; bounds(r.x,r.y,r.w,r.h);
}
void gfx_recorte(float x, float y, float w, float h) { bounds(x,y,w,h); }
void gfx_sem_recorte(void) {}
void ponteiro_alvo(float x, float y, float w, float h, PonteiroFn focus, PonteiroFn activate, int i, int b) {
  (void)activate; (void)b; bounds(x,y,w,h);
  if (focus == ptFocoEmail) { assert(i >= 0 && i < LE_N); labels[i]++; }
}
TxtLinha txt_linha(TxtEstilo style, const char *s, int r, int g, int b, int a) {
  (void)style; (void)r; (void)g; (void)b; (void)a;
  return (TxtLinha){.w = (int)strlen(s) * 11, .h = 25};
}
TxtLinha txt_linha_corta(TxtEstilo style, const char *s, int r, int g, int b, int a, float max) {
  TxtLinha l = txt_linha(style,s,r,g,b,a); if (l.w > max) l.w = (int)max; return l;
}
void txt_desenhar_alpha(TxtLinha l, float x, float y, float a) {
  (void)a;
  /* Long field values intentionally scroll inside the production scissor. */
  if (x >= 0) { assert(y + l.h <= NV_TELA_H); if (y + l.h > bottom) bottom = y + l.h; }
}
float txt_bloco_corta(TxtEstilo style, const char *s, int r, int g, int b, float x, float y, float w, float leading, float a, int lines) {
  (void)style; (void)s; (void)r; (void)g; (void)b; (void)a;
  bounds(x,y,w,leading * lines); return leading * lines;
}
static void key(SDL_Keycode k) {
  SDL_Event e = {0}; e.type = SDL_KEYDOWN; e.key.keysym.sym = k; login_evento(&e);
}
static void enter(const char *text) {
  snprintf(keyboardText, sizeof keyboardText, "%s", text);
  keyboardResult = TECLADO_PRONTO; keyboardOpen = 0; login_atualizar(0.016f, 1);
}
int main(void) {
  login_iniciar(); assert(!cadastro && foco == LE_EMAIL);
  key(SDLK_DOWN); assert(foco == LE_SENHA);
  key(SDLK_RIGHT); assert(foco == LE_MOSTRAR);
  key(SDLK_LEFT); assert(foco == LE_SENHA);
  key(SDLK_DOWN); assert(foco == LE_ENTRAR);
  key(SDLK_DOWN); assert(foco == LE_ALTERNAR);
  key(SDLK_RETURN); assert(cadastro && foco == LE_EMAIL);
  key(SDLK_RETURN); assert(keyboardOpen && !strcmp(keyboardTitle,"E-mail"));
  enter("fixture@example.invalid"); assert(keyboardOpen && !strcmp(keyboardTitle,"Senha"));
  enter("Senha-Teste9"); assert(keyboardOpen && !strcmp(keyboardTitle,"Confirmar senha") && masked);
  enter("Diferente9"); assert(!keyboardOpen && foco == LE_ENTRAR && !keyboardText[0]);
  key(SDLK_RETURN); assert(!requestCount && foco == LE_CONFIRMAR && strstr(aviso,"coincidem"));
  key(SDLK_RETURN); enter("Senha-Teste9"); key(SDLK_RETURN);
  assert(requestCount == 1 && requestSignup && esperando && !senha[0] && !confirmar[0]);
  key(SDLK_RETURN); key(SDLK_ESCAPE); assert(requestCount == 1 && cadastro);
  status = SES_CONFIRMAR_EMAIL; login_atualizar(0.016f, 2);
  assert(!cadastro && avisoInformativo && strstr(aviso,"caixa de e-mail") && foco == LE_SENHA);
  key(SDLK_RETURN); enter("Senha-Teste9"); key(SDLK_RETURN);
  assert(requestCount == 2 && !requestSignup && !strcmp(sentPassword,"Senha-Teste9"));
  status = SES_DESLOGADO; sessionError = "E-mail ou senha incorretos.";
  login_atualizar(0.016f, 3); assert(foco == LE_SENHA && mostrarFalha);
  ptFocoEmail(LE_ALTERNAR,0); key(SDLK_RETURN);
  assert(cadastro && !mostrarFalha && !aviso[0]);
  strcpy(email,"bad"); strcpy(senha,"abcdef"); strcpy(confirmar,"abcdef");
  enviar(); assert(foco == LE_EMAIL && requestCount == 2);
  strcpy(email,"fixture@example.invalid"); strcpy(senha,"123");
  enviar(); assert(foco == LE_SENHA && requestCount == 2);
  strcpy(senha,"123456"); strcpy(confirmar,"123456"); key(SDLK_ESCAPE);
  assert(!cadastro && !senha[0] && !confirmar[0]);
  assert(!emailValido("x@@example.invalid") && !emailValido(" x@example.invalid"));
  assert(emailValido("x+y@example.invalid"));
  /* Ambos formularios cabem de 85% a 150% da escala TV, com erro de 2 linhas. */
  const float heights[] = {720, 864, 1080, 1270};
  for (size_t i=0;i<sizeof heights/sizeof *heights;i++) {
    nv_layout_h=heights[i]; nv_layout_w=heights[i]*16/9;
    for (cadastro=0;cadastro<2;cadastro++) {
      memset(labels,0,sizeof labels); bottom=0;
      for (int j=0;j<LE_N;j++) animFoco[j]=0;
      snprintf(aviso,sizeof aviso,"Confira os dados e tente novamente.");
      painelLoginStreamMax();
      desenharEmail(lgMedida(118)+tituloLogin(lgMedida(118)),0);
      assert(labels[LE_EMAIL] && labels[LE_SENHA] && labels[LE_ENTRAR] && labels[LE_ALTERNAR]);
      assert((labels[LE_CONFIRMAR] != 0) == cadastro);
      assert(bottom < NV_TELA_H);
    }
  }
  puts("OK: controle, ponteiro, teclado, validacao, troca de modo e limites 85-150%");
}
