// trailer.c inteiro. Backend controlado registra o PRIMEIRO comando enviado;
// testes anteriores iniciavam nativoJanela ja com dono=HOME e nao cobriam abrir.
#include "trailer.h"
#include "video.h"
#include "ajustes.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
float nv_layout_w=1920,nv_layout_h=1080;
static Uint32 ticks=100;
static int ready,playing,width,height,shouldFail;
static int modes[128],nMode,atomicOpen;
static void record(int mode) { assert(nMode<128);modes[nMode++]=mode; }
Uint32 sm_test_ticks(void){return ticks;}
int layout_modo_mobile(void){return 0;}
int ajustes_home_layout(void){return HOME_LAYOUT_MODERNA;}
float ajustes_trailer_zoom(void){return 1;}
int trailerfonte_tizen(void){return 0;}
int trailerfonte_com_som(int tizen){return !tizen;}
int video_iniciar_auto(void){return 1;}
int video_tocar(const char *url){assert(url&&*url);return !shouldFail;}
int video_android_tocar_home(const char *url,int x,int y,int w,int h){
 assert(w>0&&h>0&&x>=0&&y<0);
 atomicOpen++;record(2);return video_tocar(url);
}
void video_janela(int x,int y,int w,int h){(void)x;(void)y;(void)w;(void)h;record(1);}
void video_android_janela_preencher(int x,int y,int w,int h){(void)x;(void)y;assert(w>0&&h>0);record(2);}
void video_janela_fonte(int sx,int sy,int sw,int sh,int x,int y,int w,int h){
 (void)sx;(void)sy;(void)sw;(void)sh;(void)x;(void)y;(void)w;(void)h;record(0);
}
void video_recorte_reaplicar(void){}
int video_recorte_fonte(void){return 1;}
int video_pronto(void){return ready;}
int video_quadro_visto(void){return ready;}
int video_tocando(void){return playing;}
int video_ativo(void){return 1;}
int video_largura(void){return width;}
int video_altura(void){return height;}
void video_volume(int pct){assert(pct==0||pct==100);}
void video_parar(void){ready=playing=0;}
void video_bombear(void){}
int video_falhou(void){return 0;}
int video_terminou(void){return 0;}
int video_conflito_recurso(void){return 0;}
static const GfxRect box={537.6f,0,1382.4f,670};
int main(void) {
#ifdef SM_TRAILER_BASELINE
 trailer_no_cartao(0);trailer_abrir("https://test/trailer.mp4",box,0,0);
 trailer_marcar_dono(TRAILER_DONO_HOME,"tt1");
 assert(nMode>0 && modes[0]==1 && atomicOpen==0);
 puts("REPRODUZIDO 1.0.23: o primeiro comando da abertura da Home e FIT, antes da marca de dono");
#else
 for(int hot=0;hot<2;hot++) {
  nMode=atomicOpen=0;ready=playing=hot;width=hot?1920:0;height=hot?1080:0;
  trailer_abrir_home(hot?"https://test/hot.mp4":"https://test/cold.mp4",box,0,0,"tt1");
  assert(trailer_aberto()&&trailer_dono()==TRAILER_DONO_HOME);
  assert(!strcmp(trailer_dono_imdb(),"tt1"));
  assert(atomicOpen==1&&nMode>0&&modes[0]==2);
  // Metadata, primeiro quadro, atualizacao de caixa e reconsulta do recorte.
  for(int i=0;i<20;i++) {
   ticks+=100;ready=playing=1;width=i<10?1920:1920;height=i<10?1080:804;
   trailer_atualizar(ticks);
   if(i==12)trailer_rect((GfxRect){538,0,1382,670});
  }
  for(int i=0;i<nMode;i++)assert(modes[i]==2);
  trailer_fechar();assert(!trailer_aberto()&&trailer_dono()==TRAILER_DONO_NENHUM);
 }
 nMode=atomicOpen=0;ready=playing=0;width=height=0;
 trailer_abrir_home("https://test/card.mp4",(GfxRect){400,400,384,216},0,1,"tt2");
 assert(trailer_aberto()&&nMode>0&&modes[0]==1&&atomicOpen==0);
 trailer_fechar();
 nMode=atomicOpen=0;shouldFail=1;
 trailer_abrir_home("https://test/fail.mp4",box,0,0,"tt3");
 assert(!trailer_aberto()&&trailer_dono()==TRAILER_DONO_NENHUM&&!*trailer_dono_imdb());
 shouldFail=0;
 nMode=atomicOpen=0;trailer_abrir("https://test/detail.mp4",box,0,0);
 trailer_marcar_dono(TRAILER_DONO_DETALHE,"tt4");
 assert(nMode>0&&modes[0]==1&&atomicOpen==0);
 trailer_abrir_home("https://test/no-steal.mp4",box,0,0,"tt5");
 assert(trailer_dono()==TRAILER_DONO_DETALHE&&!strcmp(trailer_dono_imdb(),"tt4"));
 trailer_fechar();
 puts("OK: trailer.c inteiro: abertura fria/quente, primeiro comando COVER, metadata tardia e nenhum FIT na Home");
 puts("OK: cartao/detalhe preservados, falha limpa contexto e Home nao toma trailer ativo");
#endif
}
