#include "addoninst.h"
#include "addonmanifest.h"
#include "rede.h"
#include "perfisessao.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct { uint64_t geracao, sessao; char url[NV_ADDON_URL_MAX]; RedeJob *job; } Pedido;
static pthread_mutex_t trava = PTHREAD_MUTEX_INITIALIZER;
static uint64_t geracao;
static RedeJob *atual;
static int rodando, pronto;
static AddonInstResultado resultado;
void addoninst_liberar(AddonInstResultado *r) { if (r) { free(r->manifesto); memset(r, 0, sizeof *r); } }
void addoninst_cancelar(void) {
  pthread_mutex_lock(&trava);
  geracao++; rodando = pronto = 0;
  if (atual) { rede_job_cancelar(atual); rede_job_soltar(atual); atual = NULL; }
  addoninst_liberar(&resultado);
  pthread_mutex_unlock(&trava);
}
static int perfilMudou(void *u) { return !nv_perfil_sessao_atual(((Pedido *)u)->sessao); }
static void *validar(void *u) {
  Pedido *p = u;
  char url[NV_ADDON_PEDIDO_MAX];
  AddonInstResultado r = { .sessao = p->sessao };
  RedeResposta resposta = {0};
  snprintf(r.url, sizeof r.url, "%s", p->url);
  int escrito = nv_addon_url(url, sizeof url, p->url, "/manifest.json");
  if (!NV_COUBE(escrito, url)) {
    snprintf(r.erro, sizeof r.erro, "O endereço do addon é muito longo.");
  } else {
    RedePedido pedido = { .url = url, .prazo_ms = 12000, .max_bytes = ADDON_MANIFESTO_MAX,
      .seguir = 1, .job = p->job, .parar = perfilMudou, .parar_usuario = p };
    if (!rede_pedir(&pedido, &resposta) || resposta.erro != REDE_OK) {
      snprintf(r.erro, sizeof r.erro, "Não foi possível consultar o addon. Confira a conexão e tente novamente.");
    } else if (resposta.status < 200 || resposta.status >= 300) {
      snprintf(r.erro, sizeof r.erro, "O addon recusou o endereço ou está indisponível. Confira o link configurado.");
    } else {
      AddonManifestoEstado estado = addonmanifest_validar(resposta.corpo, resposta.n_corpo, r.nome, sizeof r.nome);
      if (estado == AM_CONFIGURAR) snprintf(r.erro, sizeof r.erro, "Configure o addon no provedor e copie o link do manifesto configurado.");
      else if (estado != AM_OK) snprintf(r.erro, sizeof r.erro, "O endereço não retornou um manifesto válido. Use o link manifest.json do addon.");
      else { r.manifesto = resposta.corpo; resposta.corpo = NULL; }
    }
  }
  rede_resposta_limpar(&resposta);
  pthread_mutex_lock(&trava);
  if (p->geracao == geracao && rede_job_estado(p->job) == REDE_OK && !perfilMudou(p)) {
    resultado = r; r.manifesto = NULL; pronto = 1; rodando = 0;
    if (atual) { rede_job_soltar(atual); atual = NULL; }
  } else if (p->geracao == geracao) {
    rodando = 0;
    if (atual) { rede_job_soltar(atual); atual = NULL; }
  }
  pthread_mutex_unlock(&trava);
  addoninst_liberar(&r); rede_job_soltar(p->job); free(p);
  return NULL;
}
int addoninst_iniciar(const char *url, uint64_t sessao) {
  pthread_t fio;
  addoninst_cancelar();
  if (!url || !*url || strlen(url) >= NV_ADDON_URL_MAX) return 0;
  Pedido *p = calloc(1, sizeof *p);
  RedeGrupo *grupo = rede_grupo_criar();
  if (!p || !grupo) { free(p); if (grupo) rede_grupo_soltar(grupo); return 0; }
  p->job = rede_job_criar(grupo); rede_grupo_soltar(grupo);
  if (!p->job) { free(p); return 0; }
  p->sessao = sessao; snprintf(p->url, sizeof p->url, "%s", url);
  pthread_mutex_lock(&trava);
  p->geracao = geracao; atual = p->job; rede_job_reter(atual); rodando = 1;
  int erro = pthread_create(&fio, NULL, validar, p);
  if (erro) {
    rede_job_soltar(atual); atual = NULL; rodando = 0;
    rede_job_soltar(p->job); free(p);
  } else pthread_detach(fio);
  pthread_mutex_unlock(&trava);
  return !erro;
}
int addoninst_rodando(void) { pthread_mutex_lock(&trava); int r = rodando; pthread_mutex_unlock(&trava); return r; }
int addoninst_resultado(AddonInstResultado *saida) {
  int r;
  pthread_mutex_lock(&trava);
  r = rodando ? 0 : -1;
  if (pronto && saida) { *saida = resultado; memset(&resultado, 0, sizeof resultado); pronto = 0; r = 1; }
  pthread_mutex_unlock(&trava);
  return r;
}
