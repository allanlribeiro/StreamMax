// Exercita o backend real (rede, jobs, decode, recorte e cache). Apenas a GPU
// e substituida: registra o pixel enviado, sem depender de janela/emulador.
#include "../src/seekr.c"
#include <assert.h>
#undef glTexImage2D
int perftv_enabled,perftv_uploads;
static GLuint bound,nextTexture;
static unsigned char colors[32][4];
void glGenTextures(GLsizei n,GLuint *textures){while(n--)*textures++=++nextTexture;}
void glBindTexture(GLenum target,GLuint texture){(void)target;bound=texture;}
void glTexParameteri(GLenum target,GLenum name,GLint value){(void)target;(void)name;(void)value;}
void glTexImage2D(GLenum target,GLint level,GLint internal,GLsizei w,GLsizei h,GLint border,GLenum format,GLenum type,const void *pixels){
  (void)target;(void)level;(void)internal;(void)border;(void)format;(void)type;
  assert(w==320&&h==180&&bound<32);memcpy(colors[bound],(const unsigned char*)pixels+((h/2)*w+w/2)*4,4);
}
void gfx_tex_esquecer(GLuint texture){(void)texture;}
static void ready(int expected){
  Uint32 limit=SDL_GetTicks()+7000;
  while(seekr_estado()==SEEKR_BUSCANDO && (Sint32)(limit-SDL_GetTicks())>0)SDL_Delay(5);
  assert(seekr_estado()==expected);
}
static GLuint frame(double position){
  GLuint t[1]={0};double cue[1];Uint32 limit=SDL_GetTicks()+5000;
  do{seekr_quadros(position,1,t,cue);if(t[0]&&!seekr_quadro_velho())return t[0];SDL_Delay(5);}while((Sint32)(limit-SDL_GetTicks())>0);
  assert(!"frame timeout");return 0;
}
int main(int argc,char **argv){
  assert(argc==3);char media[MINI_URL],other[MINI_URL];SeekrVtt v={0};
  assert(SDL_Init(SDL_INIT_TIMER)==0);IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG);
  snprintf(media,sizeof media,"%s/redirect/master.m3u8",argv[1]);
  const char *auth="Authorization: Bearer fixture\nX-Source-Key: fixture\nHost: forbidden.example";
  RedeGrupo *group=rede_grupo_criar();RedeJob *job=rede_job_criar(group);
  assert(minifonte_carregar(media,"",auth,job,&v)==1);
  assert(v.nCues==3&&v.cues[2].ini==20000&&v.cues[2].fim==25000);
  assert(strstr(v.folhas[0],"/catalog/thumbs/sheet.png"));seekr_vtt_liberar(&v);
  assert(minifonte_carregar(media,"/jump.vtt",auth,job,&v)==1);
  snprintf(other,sizeof other,"%s/public/sheet.png",argv[2]);assert(!strcmp(v.folhas[0],other));
  long bytes;int http;char *image=minifonte_baixar(other,media,auth,job,&bytes,&http);
  assert(image&&http==200&&bytes>20);free(image);seekr_vtt_liberar(&v);
  assert(minifonte_carregar(media,"/large.vtt",auth,job,&v)==-1);
  rede_job_cancelar(job);assert(minifonte_carregar(media,"/never.vtt",auth,job,&v)==-1);
  rede_job_soltar(job);rede_grupo_soltar(group);
  seekr_fonte_pedir(media,"",auth);ready(SEEKR_PRONTO);
  GLuint red=frame(2);assert(colors[red][0]>240&&colors[red][1]<10&&colors[red][2]<10);
  GLuint green=frame(12);assert(colors[green][0]<10&&colors[green][1]>240&&colors[green][2]<10);
  GLuint blue=frame(22);assert(colors[blue][0]<10&&colors[blue][1]<10&&colors[blue][2]>240);
  GLuint strip[3];double times[3];assert(seekr_quadros(12,3,strip,times));
  assert(strip[0]==red&&strip[1]==green&&strip[2]==blue&&times[0]==0&&times[1]==10&&times[2]==20);
  seekr_ocioso();pthread_mutex_lock(&trava);assert(!folhaSup&&jpgBytes);pthread_mutex_unlock(&trava);
  // Mesma fonte nao baixa a timeline novamente.
  unsigned g=geracao;seekr_fonte_pedir(media,"",auth);assert(geracao==g);
  // Resposta atrasada da fonte anterior nunca publica quadros na nova.
  seekr_fonte_pedir(media,"/slow.vtt",auth);SDL_Delay(60);
  seekr_fonte_pedir(media,"/plain.vtt",auth);ready(SEEKR_PRONTO);
  red=frame(2);assert(colors[red][0]>240&&colors[red][2]<10);
  seekr_fonte_pedir(media,"/slow-image.vtt",auth);ready(SEEKR_PRONTO);
  GLuint pending;seekr_quadros(2,1,&pending,NULL);seekr_ocioso();
  for(int k=0;k<1000;k++){
    pthread_mutex_lock(&trava);int busy=emVoo;pthread_mutex_unlock(&trava);
    if(!busy)break;
    SDL_Delay(5);
  }
  pthread_mutex_lock(&trava);assert(!emVoo&&!folhaSup&&jpgBytes);pthread_mutex_unlock(&trava);
  seekr_fonte_pedir(media,"/bad-image.vtt",auth);ready(SEEKR_PRONTO);
  for(int k=0;k<100&&seekr_estado()==SEEKR_PRONTO;k++){GLuint t;seekr_quadros(2,1,&t,NULL);SDL_Delay(5);}
  assert(seekr_estado()==SEEKR_SEM_PREVIA);
  snprintf(media,sizeof media,"%s/never.mp4",argv[1]);seekr_fonte_pedir(media,"",auth);ready(SEEKR_SEM_PREVIA);
  seekr_desligar();assert(seekr_estado()==SEEKR_DESLIGADO&&!seekr_da_fonte());
  puts("OK: rede real, credenciais, redirects, cancelamento, recortes RGB, cache e troca de fonte");
  return 0;
}
