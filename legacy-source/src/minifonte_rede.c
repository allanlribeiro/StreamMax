#include "minifonte_rede.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// Source credentials belong only to the original media origin. Redirects are
// handled by rede_pedir, which drops every caller header across origins.
static void cabecalhos(const char *raw,char lines[16][512],const char *list[17]) {
  int n=0;list[0]=NULL;
  for(const char *p=raw;p && *p && n<16;) {
    const char *end=strchr(p,'\n');if(!end)end=p+strlen(p);
    const char *colon=memchr(p,':',(size_t)(end-p));int valid=colon && colon>p && end-p<512;
    for(const char *q=p;valid && q<end;q++) {
      if(q<colon)valid=isalnum((unsigned char)*q)||strchr("!#$%&'*+-.^_`|~",*q)!=NULL;
      else valid=(unsigned char)*q>=32 && (unsigned char)*q!=127;
    }
    if(valid) {
      size_t k=(size_t)(colon-p);
      static const char *const blocked[]={"Host","Range","Accept-Encoding","Content-Length","Connection","Transfer-Encoding"};
      for(unsigned i=0;i<sizeof blocked/sizeof *blocked;i++)if(strlen(blocked[i])==k&&!strncasecmp(p,blocked[i],k))valid=0;
    }
    if(valid){memcpy(lines[n],p,(size_t)(end-p));lines[n][end-p]=0;list[n]=lines[n];n++;list[n]=NULL;}
    p=*end?end+1:end;
  }
}
static int buscar(const char *url,const char *video,const char *cab,RedeJob *job,size_t max,RedeResposta *r) {
  char lines[16][512];const char *headers[17]={NULL};
  if(minifonte_mesma_origem(url,video))cabecalhos(cab,lines,headers);
  RedePedido p={.url=url,.cabecalhos=headers,.prazo_ms=4500,.max_bytes=max,.max_cabecalhos=16384,.seguir=1,.job=job};
  return rede_pedir(&p,r) && r->erro==REDE_OK && r->status==200;
}
int minifonte_carregar(const char *video,const char *vtt,const char *cab,RedeJob *job,SeekrVtt *out) {
  char url[MINI_URL],image[MINI_URL];RedeResposta r={0};int ok=0,w=0,h=0;
  memset(out,0,sizeof *out);
  if(!(rede_pedido_capacidades()&REDE_CAP_JOB) || !video || !video[0])return 0;
  if(vtt && vtt[0]) {
    if(!minifonte_url(video,vtt,url,sizeof url))return 0;
    if(buscar(url,video,cab,job,MINI_DOCUMENTO,&r))ok=minifonte_vtt(r.corpo,r.final,out)>0;
    else ok=r.erro==REDE_OK?0:-1;
    rede_resposta_limpar(&r);return ok;
  }
  if(!minifonte_e_hls(video))return 0; // Do not probe/download arbitrary MP4/MKV files.
  if(!buscar(video,video,cab,job,MINI_DOCUMENTO,&r)){ok=r.erro==REDE_OK?0:-1;goto fim;}
  if(minifonte_hls_imagens(r.corpo,r.final,0,0,out)>0){ok=1;goto fim;}
  if(!minifonte_hls_indice(r.corpo,r.final,image,sizeof image,&w,&h))goto fim;
  rede_resposta_limpar(&r);
  if(buscar(image,video,cab,job,MINI_DOCUMENTO,&r))ok=minifonte_hls_imagens(r.corpo,r.final,w,h,out)>0;
  else ok=r.erro==REDE_OK?0:-1;
fim:rede_resposta_limpar(&r);return ok;
}
char *minifonte_baixar(const char *url,const char *video,const char *cab,RedeJob *job,long *n,int *http) {
  RedeResposta r={0};char *body=NULL;*n=0;*http=0;
  if(buscar(url,video,cab,job,MINI_IMAGEM,&r)){body=r.corpo;r.corpo=NULL;*n=(long)r.n_corpo;}
  *http=r.status;rede_resposta_limpar(&r);return body;
}
