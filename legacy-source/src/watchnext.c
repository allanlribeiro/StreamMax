#ifdef NV_ANDROID
#include "watchnext.h"
#include "catalogo.h"
#include "progresso.h"
#include "perfis.h"
#include "sessao.h"
#include "ajustes.h"
#include "deeplink.h"
#include <SDL2/SDL.h>
#include <jni.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WN_MAX 12
#define WN_JSON_MAX (128 * 1024)
static atomic_int sujo = 1;
void watchnext_sujar(void) { atomic_store(&sujo, 1); }

typedef struct { char *texto; size_t n; int erro; } Json;
static void texto(Json *j, const char *s) {
  size_t n = strlen(s);
  if (j->erro || j->n + n >= WN_JSON_MAX) { j->erro = 1; return; }
  memcpy(j->texto + j->n, s, n + 1); j->n += n;
}
static void cadeia(Json *j, const char *s) {
  texto(j, "\"");
  for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
    char b[8] = {0};
    if (*p < 32) snprintf(b, sizeof b, "\\u%04x", *p);
    else if (*p == '"' || *p == '\\') { b[0] = '\\'; b[1] = (char)*p; }
    else b[0] = (char)*p;
    texto(j, b);
  }
  texto(j, "\"");
}
static void campo(Json *j, const char *nome, const char *valor) {
  cadeia(j, nome); texto(j, ":"); cadeia(j, valor);
}
static int enviar(const char *json) {
  JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
  jobject act = SDL_AndroidGetActivity();
  if (!env || !act) return 0;
  jclass cls = (*env)->GetObjectClass(env, act);
  jmethodID metodo = cls ? (*env)->GetMethodID(env, cls, "publicarContinuarAssistindo", "([B)V") : NULL;
  jbyteArray dados = NULL;
  if (metodo) {
    size_t n = strlen(json);
    dados = (*env)->NewByteArray(env, (jsize)n);
    if (dados) {
      (*env)->SetByteArrayRegion(env, dados, 0, (jsize)n, (const jbyte *)json);
      (*env)->CallVoidMethod(env, act, metodo, dados);
    }
  }
  int ok = metodo && dados && !(*env)->ExceptionCheck(env);
  if ((*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
  if (dados) (*env)->DeleteLocalRef(env, dados);
  if (cls) (*env)->DeleteLocalRef(env, cls);
  (*env)->DeleteLocalRef(env, act);
  return ok;
}

/* Called on the SDL thread, after progress checkpoints. No HTTP or provider
 * Binder calls here: Kotlin publishes on its serial background worker. */
void watchnext_atualizar(unsigned agora, int pronto) {
  static unsigned ultimo;
  static uint64_t sessaoAnterior;
  static int estavaPronto = -1;
  uint64_t sessao = perfis_sessao_geracao();
  pronto = pronto && sessao_logada() && !perfis_precisa_escolher();
  if (sessao == sessaoAnterior && pronto == estavaPronto &&
      !atomic_load(&sujo) && agora - ultimo < 30000u) return;
  if (!perfis_sessao_travar(sessao)) return;
  atomic_store(&sujo, 0);
  sessaoAnterior = sessao; estavaPronto = pronto; ultimo = agora;
  char *buf = calloc(1, WN_JSON_MAX);
  ProgRegistro *regs = pronto ? calloc(PROG_MAX, sizeof *regs) : NULL;
  if (!buf || (pronto && !regs)) {
    free(buf); free(regs); perfis_sessao_destravar(); return;
  }
  Json j = { .texto = buf };
  char numero[200];
  texto(&j, "{"); campo(&j, "owner", pronto ? sessao_usuario() : "");
  snprintf(numero, sizeof numero, ",\"ready\":%s,\"profile\":%d,\"session\":%llu,\"completed\":%d,\"items\":[",
           pronto ? "true" : "false", perfis_ativo(), (unsigned long long)sessao, ajustes_cw_concluido());
  texto(&j, numero);
  int n = regs ? prog_ler(regs, PROG_MAX) : 0, publicados = 0;
  for (int i = 0; i < n && publicados < WN_MAX; i++) {
    const ProgRegistro *r = &regs[i];
    int repetido = 0;
    // Only the most recent episode per title; never resurrect older episodes
    // when the newest is completed, hidden, or removed.
    for (int k = 0; k < i; k++)
      if (!strcmp(regs[k].contentId, r->contentId) && !strcmp(regs[k].tipo, r->tipo)) { repetido = 1; break; }
    if (repetido || r->lastWatchedMs <= 0 || !isfinite(r->posSeg) || !isfinite(r->durSeg) ||
        r->durSeg < 60 || r->durSeg > 2147483 || r->posSeg < 0 || r->posSeg > r->durSeg ||
        prog_removido_vence(r->contentId, r->lastWatchedMs) || prog_oculto_vence(r->contentId, r->lastWatchedMs)) continue;
    int indice = cat_indice_por_imdb_tipo(r->contentId, r->tipo);
    const CatItem *c = indice >= 0 ? cat_item(indice) : NULL;
    if (!c || !c->titulo[0]) continue;
    char link[256]; DeepLink dl;
    int serie = !strcmp(r->tipo, "series");
    double inicio = serie ? 120.0 : fmin(120.0, r->durSeg * 0.03);
    int pct = ajustes_cw_concluido();
    if (pct < 50) pct = 50;
    if (pct > 99) pct = 99;
    double fim = r->durSeg * pct / 100.0;
    if (serie && r->durSeg > 600.0) fim = fmin(fim, r->durSeg - 180.0);
    if (r->posSeg <= inicio || r->posSeg >= fim) continue;
    snprintf(link, sizeof link, "streammax://play/%s/%s", serie ? "series" : "movie", r->contentId);
    if (serie && r->temporada > 0 && r->episodio > 0)
      snprintf(link, sizeof link, "streammax://play/series/%s/%d/%d", r->contentId, r->temporada, r->episodio);
    if (!deeplink_parse(link, &dl)) continue;  // Private/server IDs are not exported.
    if (publicados++) texto(&j, ",");
    texto(&j, "{"); campo(&j, "id", r->contentId); texto(&j, ","); campo(&j, "title", c->titulo);
    texto(&j, ","); campo(&j, "art", c->backdrop[0] ? c->backdrop : c->poster);
    texto(&j, ","); campo(&j, "episodeTitle", c->nomeEpisodio);
    snprintf(numero, sizeof numero,
        ",\"series\":%s,\"season\":%d,\"episode\":%d,\"position\":%lld,\"duration\":%lld,\"engaged\":%lld}",
        serie ? "true" : "false", r->temporada, r->episodio,
        (long long)(r->posSeg * 1000), (long long)(r->durSeg * 1000), r->lastWatchedMs);
    texto(&j, numero);
  }
  texto(&j, "]}");
  if (!j.erro) enviar(buf);
  free(regs); free(buf);
  perfis_sessao_destravar();
  // Even a failed JNI handoff retries at the normal interval, never every frame.
}

JNIEXPORT void JNICALL Java_space_nuvio_nativelegacy_NuvioActivity_nativeWatchNextRefresh(JNIEnv *env, jobject act) {
  (void)env; (void)act; watchnext_sujar();
}
#endif
