// Fixture de estado: legsync.c completo; coletores/engine externos controlados.
#include <assert.h>
#include "../src/legsync.c"
static int oldOffset=1700, paused, selected;
int autosync_offset_ms(AutoSync *s,int slot){(void)s;(void)slot;return oldOffset;}
double autosync_posicao(AutoSync *s,int slot,double p){(void)s;(void)slot;return p*1.02+3;}
int autosync_selecionar(AutoSync *s,int slot,LegendaDocumento *d){(void)s;(void)slot;(void)d;selected++;return 1;}
void autosync_iniciar(AutoSync *s,uint64_t session){(void)s;(void)session;}
void legref_pausar(LegRef *r,int p){(void)r;paused=p;}
void legref_cancelar(LegRef *r){(void)r;}
void audsync_cancelar(void){}
void legenda2_definir_offset_auto(int ms){(void)ms;}
int main(void) {
  const char *url="https://example.invalid/movie";
  LegendaCue cue={.inicio=10,.fim=12,.texto="Uma frase completa."};
  LegendaDocumentoInfo info={.sessao=9,.flags=LEGENDA_DOC_COMPLETO};
  LegendaDocumento *d=legenda_documento_de_cues(&cue,1,&info);
  L.criado=1;L.sessao=9;L.urlHash=fnv(url);L.primTipo=1;L.primFase=1;
  L.primGer=legenda_geracao();L.primDoc=d;L.ref=(LegRef*)1;
  unsigned g=99;LegendaDocumento *snapshot=legsync_fala_documento(&g);
  assert(snapshot==d && g==L.primGer);
  assert(legsync_offset_ms(400)==1300 && legsync_posicao(10)==13.2);
  assert(!legsync_fala_aplicar(d,g+1,url,400,2000));
  assert(!legsync_fala_aplicar(d,g,"https://example.invalid/other",400,2000));
  assert(!legsync_fala_aplicar(d,g,url,0,60001));
  assert(legsync_fala_aplicar(d,g,url,400,2000) && paused);
  assert(legsync_offset_ms(400)==-2000 && legsync_posicao(10)==10);
  assert(legsync_offset_ms(500)==-2100); // existing fine adjustment still adds 100ms
  oldOffset=-5400; // a late automatic result cannot overwrite the user's choice
  assert(legsync_offset_ms(400)==-2000 && legsync_posicao(10)==10);
  assert(legsync_fala_aplicar(d,g,url,400,-1300));
  assert(legsync_offset_ms(400)==1300);
  assert(legsync_fala_pode_desfazer(url));
  assert(!legsync_fala_desfazer("https://example.invalid/other"));
  assert(legsync_fala_desfazer(url) && !legsync_fala_pode_desfazer(url));
  assert(legsync_offset_ms(400)==-5800 && legsync_posicao(10)==13.2);
  assert(legsync_fala_aplicar(d,g,url,0,1000));
  L.sessao++;assert(!legsync_fala_valida(d,g,url));L.sessao--;
  L.primGer++;assert(!legsync_fala_valida(d,g,url));L.primGer--;
  novaSessao("https://example.invalid/next-source",1);
  assert(!L.falaAtiva && !L.falaUndo);
  assert(!legsync_fala_valida(snapshot,g,url));
  assert(!legsync_fala_valida(snapshot,g,"https://example.invalid/next-source"));
  legsync_primaria_outra(1);
  assert(!L.falaAtiva && !L.falaUndo && selected);
  assert(!legsync_fala_valida(snapshot,g,url));
  assert(legsync_offset_ms(600)==-600 && legsync_posicao(10)==10);
  legenda_documento_liberar(snapshot);
  puts("OK: ponte real: geracao/fonte, sinal, auto tardio, ajuste fino e desfazer");
}
