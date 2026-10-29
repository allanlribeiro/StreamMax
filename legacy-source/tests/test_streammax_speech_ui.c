#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "legfala.h"
#include "legsync.h"
#include "legendasui.h"
#include "plrui.h"
#include "text.h"
#include "ponteiro.h"
#include "layout.h"
#include "player.h"
#include "idioma.h"
static int falaAberta,menu,okPendente,okLongo;
static char aviso[200],lastFull[1536];
static void avisar(const char *s){snprintf(aviso,sizeof aviso,"%s",s);}
static void falaAtualizar(void);
#include "../src/legendasui_fala.inc"
static LegendaDocumento *doc;
static unsigned generation=4;
static float position=10.3f;
static VideoLegendaEstilo style;
static const char *url="https://example.invalid/movie",*channel="";
static int applied,delayApplied;
float nv_layout_w=1920,nv_layout_h=1080;
const char *i18n(const char *s){return s;}
Uint32 SDL_GetTicks(void){return 1234;}
const char *video_url_atual(void){return url;}
const char *player_id_canal(void){return channel;}
float player_posicao_seg(void){return position;}
VideoLegendaEstilo *player_leg_estilo(void){return &style;}
LegendaDocumento *legsync_fala_documento(unsigned *g){*g=generation;return legenda_documento_reter(doc);}
int legsync_fala_valida(const LegendaDocumento *d,unsigned g,const char *u){return d && d==doc && g==generation && !strcmp(u,"https://example.invalid/movie");}
int legsync_fala_aplicar(const LegendaDocumento *d,unsigned g,const char *u,int manual,int total){(void)manual;if(!legsync_fala_valida(d,g,u))return 0;applied++;delayApplied=total;return 1;}
double legsync_posicao(double p){return p;}
int legsync_offset_ms(int m){return -m;}
static void bounds(GfxRect r){assert(r.x>=0 && r.y>=0 && r.w>0 && r.h>0 && r.x+r.w<=nv_layout_w+.1f && r.y+r.h<=nv_layout_h+.1f);}
void plrui_linha_foco(GfxRect r,float raio,float a){(void)raio;(void)a;bounds(r);}
void gfx_cor(GfxRect r,float raio,float cr,float cg,float cb,float a){(void)raio;(void)cr;(void)cg;(void)cb;(void)a;bounds(r);}
void gfx_recorte(float x,float y,float w,float h){bounds((GfxRect){x,y,w,h});}
void gfx_sem_recorte(void){}
void ponteiro_alvo(float x,float y,float w,float h,PonteiroFn f,PonteiroFn a,int i,int token){(void)f;(void)a;(void)i;(void)token;bounds((GfxRect){x,y,w,h});}
TxtLinha txt_linha_corta(TxtEstilo e,const char *s,int r,int g,int b,int a,float w){(void)e;(void)r;(void)g;(void)b;(void)a;return (TxtLinha){.w=(int)fminf(w,strlen(s)*12),.h=25};}
void txt_desenhar_alpha(TxtLinha t,float x,float y,float a){(void)t;(void)x;(void)y;(void)a;}
int txt_largura(TxtEstilo e,const char *s){(void)e;return (int)strlen(s)*13;}
float txt_bloco(TxtEstilo e,const char *s,int r,int g,int b,float x,float y,float w,float lead,float a,int max){
  (void)e;(void)r;(void)g;(void)b;(void)x;(void)y;(void)max;
  if(a)snprintf(lastFull,sizeof lastFull,"%s",s);
  return ceilf(strlen(s)*13/fmaxf(w,1))*lead;
}
float txt_bloco_corta(TxtEstilo e,const char *s,int r,int g,int b,float x,float y,float w,float lead,float a,int max){return txt_bloco(e,s,r,g,b,x,y,w,lead,a,max);}
static void key(SDL_Keycode k,int up){SDL_Event e={0};e.type=up?SDL_KEYUP:SDL_KEYDOWN;e.key.keysym.sym=k;falaEvento(&e);}
static void press(SDL_Keycode k){key(k,0);key(k,1);}
int main(void){
  char time[32];falaTempo(INFINITY,time,sizeof time);assert(!strcmp(time,"--:--:--"));
  falaTempo(1e200,time,sizeof time);assert(!strcmp(time,"--:--:--"));
  falaTempo(3601,time,sizeof time);assert(!strcmp(time,"01:00:01"));
  LegendaCue cues[3]={{.inicio=1,.fim=2,.texto="Primeira"},{.inicio=10,.fim=12,.texto="Frase inteira"},{.inicio=20,.fim=22,.texto="Ultima"}};
  LegendaDocumentoInfo info={.sessao=1,.flags=LEGENDA_DOC_COMPLETO};
  doc=legenda_documento_de_cues(cues,3,&info);
  falaAbrir();assert(falaAberta && !fala.capturada);
  falaDesenhar((GfxRect){48,48,1000,400},1); // transicao da ilha anterior
  key(SDLK_RETURN,0);assert(!fala.capturada && !applied); // held opening OK
  key(SDLK_RETURN,1);key(SDLK_RETURN,0);assert(fala.capturada && fala.foco==1);
  unsigned oldToken=falaTela-1;
  falaPonteiroAplicar(2,(int)oldToken);assert(!applied);
  key(SDLK_RETURN,0);assert(!applied);key(SDLK_RETURN,1); // no double capture/apply
  press(SDLK_DOWN);assert(fala.foco==2);press(SDLK_UP);assert(fala.foco==1);
  position=35; // video keeps playing while a historical capture is selected
  press(SDLK_RETURN);assert(applied==1 && delayApplied==0 && !falaAberta);
  position=10.3f;
  falaAbrir();press(SDLK_RETURN); // release opening latch first
  assert(!fala.capturada);press(SDLK_RETURN);assert(fala.capturada);
  generation++;falaAtualizar();assert(!falaAberta && strstr(aviso,"mudou"));
  falaAbrir();key(SDLK_RETURN,1);press(SDLK_RETURN);
  url="https://example.invalid/other";press(SDLK_RETURN);assert(!falaAberta && applied==1);
  url="https://example.invalid/movie";
  falaAbrir();key(SDLK_RETURN,1);press(SDLK_RETURN);press(SDLK_ESCAPE);assert(falaAberta&&!fala.capturada);
  press(SDLK_ESCAPE);assert(!falaAberta);
  falaAbrir();key(SDLK_RETURN,1);
  falaPonteiroMarcar(0,(int)falaTela);assert(fala.capturada);
  falaPonteiroAplicar(2,(int)falaTela);assert(applied==2 && !falaAberta);
  legenda_documento_liberar(doc);doc=NULL;
  falaAbrir();assert(!falaAberta && strstr(aviso,"addon"));
  // Full selected text survives beyond the one-line list preview, with scrolling.
  memset(cues[0].texto,'X',700);strcpy(cues[0].texto+700," fim completo");
  doc=legenda_documento_de_cues(cues,1,&info);position=1.3f;
  const float scales[]={720,864,1080,1270};
  for(size_t i=0;i<sizeof scales/sizeof *scales;i++){
    nv_layout_h=scales[i];nv_layout_w=scales[i]*16/9;
    GfxRect area={48,48,fminf(1420,nv_layout_w-96),fminf(880,nv_layout_h-96)};
    falaAbrir();falaDesenhar(area,1);key(SDLK_RETURN,1);press(SDLK_RETURN);
    falaDesenhar(area,1);int xs=0;for(char *p=lastFull;*p;p++)xs+=*p=='X';
    assert(xs==700 && strstr(lastFull,"completo"));
    for(int j=0;j<100;j++)press(SDLK_RIGHT);
    assert(falaScroll==falaScrollMax);falaDesenhar(area,1);
    legendasui_fala_fechar();
  }
  legenda_documento_liberar(doc);
  puts("OK: painel real, controle, ponteiro, frases completas, troca de faixa/fonte e escala");
}
