#define _POSIX_C_SOURCE 200809L
#include "addoninst.h"
#include "addonmanifest.h"
#include "rede.h"
#include <stdatomic.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
struct RedeGrupo { atomic_int refs; };
struct RedeJob { atomic_int refs, cancelado; RedeGrupo *grupo; };
static atomic_int modo, bloquear, comecou, terminou;
static atomic_uint_fast64_t sessao = 1;
static void pausa(void) { struct timespec t = {0,1000000}; nanosleep(&t,NULL); }
uint64_t perfis_sessao_geracao(void) { return atomic_load(&sessao); }
int perfis_sessao_atual(uint64_t s) { return s == atomic_load(&sessao); }
RedeGrupo *rede_grupo_criar(void) { RedeGrupo *g = calloc(1,sizeof *g); atomic_init(&g->refs,1);return g; }
void rede_grupo_soltar(RedeGrupo *g) { if (g && atomic_fetch_sub(&g->refs,1)==1) free(g); }
RedeJob *rede_job_criar(RedeGrupo *g) { RedeJob *j=calloc(1,sizeof *j);atomic_init(&j->refs,1);j->grupo=g;atomic_fetch_add(&g->refs,1);return j; }
void rede_job_reter(RedeJob *j) { atomic_fetch_add(&j->refs,1); }
void rede_job_soltar(RedeJob *j) { if (atomic_fetch_sub(&j->refs,1)==1) { rede_grupo_soltar(j->grupo);free(j); } }
void rede_job_cancelar(RedeJob *j) { atomic_store(&j->cancelado,1); }
RedeErro rede_job_estado(RedeJob *j) { return atomic_load(&j->cancelado)?REDE_CANCELADO:REDE_OK; }
int rede_pedir(const RedePedido *p,RedeResposta *r) {
  int m=atomic_load(&modo);
  assert(p->prazo_ms==12000 && p->max_bytes==ADDON_MANIFESTO_MAX && p->seguir==1);
  assert(!strcmp(p->url,"https://addon.example/AbC/manifest.json?token=XyZ"));
  atomic_fetch_add(&comecou,1);
  while (atomic_load(&bloquear) && rede_job_estado(p->job)==REDE_OK && !p->parar(p->parar_usuario)) pausa();
  if (rede_job_estado(p->job)!=REDE_OK || p->parar(p->parar_usuario)) { r->erro=REDE_CANCELADO;atomic_fetch_add(&terminou,1);return 0; }
  r->status=m==1?403:200;r->erro=m==2?REDE_TRANSPORTE:REDE_OK;
  const char *body=m==3?"<html>configurar</html>":m==4?"{\"id\":\"x\",\"name\":\"A\",\"version\":\"1\",\"resources\":[],\"behaviorHints\":{\"configurationRequired\":true}}":"{\"id\":\"x\",\"name\":\"Addon\",\"version\":\"1\",\"resources\":[\"stream\"]}";
  r->corpo=strdup(body);r->n_corpo=strlen(body);atomic_fetch_add(&terminou,1);return r->erro==REDE_OK;
}
void rede_resposta_limpar(RedeResposta *r) { free(r->corpo);memset(r,0,sizeof *r); }
static AddonInstResultado esperar(void) {
  AddonInstResultado r={0};int i;
  for(i=0;i<2000&&addoninst_resultado(&r)!=1;i++) pausa();
  assert(i<2000);return r;
}
static void iniciar(void) { assert(addoninst_iniciar("https://addon.example/AbC?token=XyZ",atomic_load(&sessao))); }
int main(void) {
  for (int m=0;m<=4;m++) {
    atomic_store(&modo,m);iniciar();AddonInstResultado r=esperar();
    assert(r.sessao==1 && !strcmp(r.url,"https://addon.example/AbC?token=XyZ"));
    assert(m==0 ? r.manifesto!=NULL && !strcmp(r.nome,"Addon") : r.erro[0] && r.manifesto==NULL);
    assert(!strstr(r.erro,"XyZ"));addoninst_liberar(&r);
  }
  atomic_store(&modo,0);atomic_store(&bloquear,1);int antes=atomic_load(&comecou);iniciar();
  for(int i=0;i<2000&&atomic_load(&comecou)==antes;i++) { pausa(); }
  assert(atomic_load(&comecou)>antes);
  addoninst_cancelar();atomic_store(&bloquear,0);
  for(int i=0;i<100;i++) { pausa(); }
  AddonInstResultado r={0};assert(addoninst_resultado(&r)==-1);
  atomic_store(&bloquear,1);antes=atomic_load(&comecou);iniciar();
  for(int i=0;i<2000&&atomic_load(&comecou)==antes;i++) { pausa(); }
  assert(atomic_load(&comecou)>antes);
  atomic_fetch_add(&sessao,1);atomic_store(&bloquear,0);
  for(int i=0;i<100;i++) { pausa(); }
  assert(addoninst_resultado(&r)==-1);assert(!addoninst_rodando());
  iniciar();r=esperar();assert(r.sessao==2 && r.manifesto);addoninst_liberar(&r);addoninst_cancelar();
  puts("OK: worker real, HTTP/HTML/configuração, tokens, cancelamento e troca de perfil");
  return 0;
}
