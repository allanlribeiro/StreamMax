#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdatomic.h>
#include <time.h>
#include "../src/sessao.c"

/* Servidor e disco controlados; todo sessao.c, JSON e scheduler sao reais.
 * Nao cria conta, nao faz rede e nao usa credenciais reais. */
static int configured = 1, replyStatus = 200, calls, writes;
static const char *reply;
static char disk[6400], pathSeen[200], bodySeen[1500];
static _Atomic int holdRequest, requestStarted;
static const char *fixtureEmail = "cadastro-fixture@example.invalid";
static const char *fixturePassword = "Fixture-Senha9!\\\"";
#define TOKEN "e30.eyJzdWIiOiJ1c3VhcmlvLXRlc3RlIiwiZXhwIjo0MTAyNDQ0ODAwfQ.c2ln"
#define TOKENS "\"access_token\":\"" TOKEN "\",\"refresh_token\":\"fixture-refresh\""

const char *i18n(const char *s) { return s; }
int nuvem_pronta(void) { return configured; }
const char *nuvem_anon(void) { return "fixture-public-key"; }
const char *nuvem_base_login(void) { return "https://example.invalid/login"; }
void nuvem_falhou(void) {}
const char *nuvem_ultimo_erro(void) { return replyStatus ? "" : "curl 28"; }
char *nuvem_post(const char *path, const char *body, const char *bearer, int *status) {
  (void)bearer;
  calls++;
  snprintf(pathSeen, sizeof pathSeen, "%s", path);
  snprintf(bodySeen, sizeof bodySeen, "%s", body);
  requestStarted = 1;
  while (holdRequest) { struct timespec t = {0, 1000000}; nanosleep(&t, NULL); }
  *status = replyStatus;
  return reply ? strdup(reply) : NULL;
}
int dados_gravar(const char *name, const char *s) {
  assert(!strcmp(name, "sessao.txt")); writes++;
  snprintf(disk, sizeof disk, "%s", s); return 1;
}
char *dados_ler(const char *name) { (void)name; return disk[0] ? strdup(disk) : NULL; }
int dados_apagar(const char *name) { (void)name; disk[0] = 0; return 1; }
void dados_sincronizar_logo(void) {}
void dados_uuid(char *dst, unsigned n) { snprintf(dst, n, "00000000-0000-4000-8000-000000000001"); }

static void waitDone(void) {
  for (int i = 0; i < 5000 && !passoPronto; i++) {
    struct timespec t = {0, 1000000}; nanosleep(&t, NULL);
  }
  assert(passoPronto);
  sessao_passo(1);
  assert(!fioVivo && !emailPedido);
  for (size_t i = 0; i < sizeof senhaPend; i++) assert(!senhaPend[i]);
  for (size_t i = 0; i < sizeof emailPend; i++) assert(!emailPend[i]);
}
static void reset(const char *body, int status) {
  assert(!fioVivo);
  sessao_sair(); erroEmail[0] = 0;
  calls = writes = 0; bodySeen[0] = pathSeen[0] = 0;
  reply = body; replyStatus = status; requestStarted = holdRequest = 0;
}
static void signup(const char *body, int status) {
  reset(body, status);
  sessao_cadastrar_email(fixtureEmail, fixturePassword);
  waitDone();
  assert(!strcmp(pathSeen, "/auth/v1/signup"));
  assert(calls == 1); /* cadastro nunca e repetido automaticamente */
  char e[256] = "", p[256] = "";
  assert(js_texto_raiz(bodySeen, "email", e, sizeof e));
  assert(js_texto_raiz(bodySeen, "password", p, sizeof p));
  assert(!strcmp(e, fixtureEmail) && !strcmp(p, fixturePassword));
  assert(!strstr(disk, fixturePassword) && !strstr(disk, fixtureEmail));
}
int main(void) {
  signup("{" TOKENS "}", 200);
  assert(sessao_logada() && sessao_estado() == SES_LOGADO && writes == 1);
  assert(!strcmp(sessao_usuario(), "usuario-teste"));
  assert(!sessao_erro_email()[0]);
  /* Mesma persistencia da sessao de login, inclusive apos reiniciar. */
  limpar(); estado = SES_DESLOGADO; sessao_iniciar();
  assert(sessao_logada() && !strcmp(sessao_usuario(), "usuario-teste"));
  char tok[3000]; assert(sessao_token_copiar(tok, sizeof tok) && !strcmp(tok, TOKEN));
  sessao_sair(); assert(!sessao_logada() && !disk[0]);

  signup("{\"session\":{" TOKENS "}}", 201);
  assert(sessao_logada() && writes == 1);
  signup("{\"id\":\"conta-fixture\",\"email\":\"cadastro-fixture@example.invalid\",\"identities\":[]}", 200);
  assert(!sessao_logada() && !writes && sessao_estado() == SES_CONFIRMAR_EMAIL);
  const char *bad[] = {"{}", "null", "", "{\"access_token\":\"broken\"}",
    "{\"access_token\":\"e30.e30.c2ln\",\"refresh_token\":\"r\"}",
    "{\"access_token\":\"" TOKEN "\"}"};
  for (size_t i = 0; i < sizeof bad / sizeof *bad; i++) {
    signup(bad[i], 200);
    assert(!sessao_logada() && !writes && sessao_estado() == SES_DESLOGADO && erroEmail[0]);
  }
  struct { int status; const char *body, *message; } errors[] = {
    {422, "{\"error_code\":\"user_already_exists\"}", "já tem uma conta"},
    {400, "{\"msg\":\"User already registered\"}", "já tem uma conta"},
    {422, "{\"error_code\":\"weak_password\"}", "Senha recusada"},
    {422, "{\"error_code\":\"email_address_invalid\"}", "endereço de e-mail"},
    {403, "{\"error_code\":\"signup_disabled\"}", "desativados"},
    {400, "{\"error_code\":\"captcha_failed\"}", "verificação adicional"},
    {429, "{}", "Muitas tentativas"}, {500, "{}", "Tente entrar"},
    {0, NULL, "Tente entrar"}, {409, "{}", "HTTP 409"}
  };
  for (size_t i = 0; i < sizeof errors / sizeof *errors; i++) {
    signup(errors[i].body, errors[i].status);
    if (!strstr(erroEmail, errors[i].message)) fprintf(stderr, "Erro de fixture %zu: %s\n", i, erroEmail);
    assert(!sessao_logada() && !writes && strstr(erroEmail, errors[i].message));
    assert(!strstr(erroEmail, fixtureEmail) && !strstr(erroEmail, fixturePassword));
  }
  reset("{" TOKENS "}", 200);
  holdRequest = 1;
  sessao_cadastrar_email(fixtureEmail, fixturePassword);
  while (!requestStarted) { struct timespec t = {0, 1000000}; nanosleep(&t, NULL); }
  assert(sessao_email_ocupada() && sessao_estado() == SES_CADASTRO);
  sessao_cadastrar_email("outro@example.invalid", "Outra-Senha9");
  sessao_login_email("outro@example.invalid", "Outra-Senha9");
  assert(calls == 1);
  holdRequest = 0; waitDone(); assert(sessao_logada() && writes == 1);

  reset("{" TOKENS "}", 200);
  sessao_login_email(fixtureEmail, fixturePassword); waitDone();
  assert(!strcmp(pathSeen, "/auth/v1/token?grant_type=password") && sessao_logada());
  reset("{\"error_code\":\"invalid_credentials\"}", 400);
  sessao_login_email(fixtureEmail, fixturePassword); waitDone();
  assert(!sessao_logada() && strstr(erroEmail, "incorretos"));
  reset(NULL, 0);
  sessao_cadastrar_email(NULL, NULL);
  sessao_cadastrar_email("bad", "123456");
  sessao_cadastrar_email(fixtureEmail, "123");
  assert(!calls && !fioVivo);
  configured = 0; sessao_cadastrar_email(fixtureEmail, fixturePassword);
  assert(!calls && strstr(erroEmail, "sem servidor"));
  puts("OK: cadastro/login, erros, persistencia, confirmacao, buffers e envio unico");
}
