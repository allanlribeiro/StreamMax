// StreamMax Windows: libmpv + GLES2/ANGLE, video abaixo da UI SDL.
// Compilado apenas no alvo NV_WINDOWS_DESKTOP. Nao afeta o APK Android.
#ifdef NV_WINDOWS_DESKTOP
#include "windows_mpv.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static mpv_handle *smPlayer;
static mpv_render_context *smRenderer;
static GLuint smFbo, smTexture, smProgram;
static int smTexW, smTexH, smDisplayW=1280, smDisplayH=720;
static int smX=0, smY=0, smW=1920, smH=1080;
static int smPlaying, smReady, smFailed, smEnded, smPaused;
static char smError[160];
static char smHeaders[2048];
static int smAudioId[8], smSubId[8];
static int smSelectedAudio=-1, smSelectedSub=-1;

static void *smGetGl(void *context, const char *name) {
  (void)context;
  return SDL_GL_GetProcAddress(name);
}
static double smDouble(const char *name) {
  double d=0;
  if (smPlayer) mpv_get_property(smPlayer,name,MPV_FORMAT_DOUBLE,&d);
  return d;
}
static int64_t smInt(const char *name, int64_t fallback) {
  int64_t n=fallback;
  if (smPlayer) mpv_get_property(smPlayer,name,MPV_FORMAT_INT64,&n);
  return n;
}
int sm_mpv_init(void) {
  if (smPlayer) return smRenderer!=NULL;
  smPlayer=mpv_create();
  if (!smPlayer) return 0;
  mpv_set_option_string(smPlayer,"vo","libmpv");
  mpv_set_option_string(smPlayer,"hwdec","auto-safe");
  mpv_set_option_string(smPlayer,"osc","no");
  mpv_set_option_string(smPlayer,"terminal","no");
  mpv_set_option_string(smPlayer,"input-default-bindings","no");
  mpv_set_option_string(smPlayer,"ytdl","no");
  if (mpv_initialize(smPlayer)<0) {
    snprintf(smError,sizeof(smError),"Falha ao inicializar libmpv");
    smFailed=1;
    mpv_terminate_destroy(smPlayer); smPlayer=NULL;
    return 0;
  }
  mpv_opengl_init_params init={smGetGl,NULL,NULL};
  mpv_render_param args[]={
    {MPV_RENDER_PARAM_API_TYPE,(void *)MPV_RENDER_API_TYPE_OPENGL},
    {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS,&init},
    {MPV_RENDER_PARAM_INVALID,NULL}
  };
  if (mpv_render_context_create(&smRenderer,smPlayer,args)<0) {
    snprintf(smError,sizeof(smError),"Renderer GLES2 do libmpv nao iniciou");
    smFailed=1; mpv_terminate_destroy(smPlayer); smPlayer=NULL;
    return 0;
  }
  return 1;
}
void sm_mpv_headers(const char *headers) {
  snprintf(smHeaders,sizeof(smHeaders),"%s",headers?headers:"");
}
int sm_mpv_play(const char *url) {
  if (!url || !*url || !sm_mpv_init()) return 0;
  smPlaying=1; smReady=smFailed=smEnded=smPaused=0;
  smError[0]=0;
  smSelectedAudio=smSelectedSub=-1;
  // Cada nova fonte apaga headers da fonte anterior; nunca registrar URLs.
  mpv_set_property_string(smPlayer,"http-header-fields",smHeaders);
  const char *cmd[]={"loadfile",url,"replace",NULL};
  if (mpv_command(smPlayer,cmd)<0) {
    smPlaying=0; smFailed=1;
    snprintf(smError,sizeof(smError),"Nao foi possivel iniciar a fonte");
    return 0;
  }
  return 1;
}
void sm_mpv_stop(void) {
  if (smPlayer) {
    const char *cmd[]={"stop",NULL};
    mpv_command(smPlayer,cmd);
  }
  smPlaying=smReady=smEnded=smPaused=0;
}
void sm_mpv_pause(int paused) {
  if (smPlayer) mpv_set_property_string(smPlayer,"pause",paused?"yes":"no");
  smPaused=paused!=0;
}
int sm_mpv_paused(void) {return smPaused;}
void sm_mpv_seek(double seconds) {
  if (!smPlayer || !smPlaying) return;
  char target[48];
  snprintf(target,sizeof(target),"%.3f", seconds<0.0?0.0:seconds);
  const char *cmd[]={"seek",target,"absolute","exact",NULL};
  mpv_command(smPlayer,cmd);
}
void sm_mpv_volume(int percent) {
  if (!smPlayer) return;
  if (percent<0) percent=0;
  if (percent>100) percent=100;
  int64_t v=percent;
  mpv_set_property(smPlayer,"volume",MPV_FORMAT_INT64,&v);
}
void sm_mpv_window(int x,int y,int w,int h) {
  smX=x;smY=y;smW=w;smH=h;
}
void sm_mpv_resize(int w,int h) {if(w>0&&h>0){smDisplayW=w;smDisplayH=h;}}
void sm_mpv_select_audio(int index) {
  if (!smPlayer || index<0 || index>=8 || !smAudioId[index]) return;
  int64_t id=smAudioId[index];
  if(mpv_set_property(smPlayer,"aid",MPV_FORMAT_INT64,&id)>=0)smSelectedAudio=index;
}
void sm_mpv_select_subtitle(int index) {
  if(!smPlayer) return;
  if(index<0){mpv_set_property_string(smPlayer,"sid","no"); smSelectedSub=-1; return;}
  if(index>=8 || !smSubId[index])return;
  int64_t id=smSubId[index];
  if(mpv_set_property(smPlayer,"sid",MPV_FORMAT_INT64,&id)>=0)smSelectedSub=index;
}
void sm_mpv_external_subtitle(const char *url) {
  if(!smPlayer || !url || !*url)return;
  const char *cmd[]={"sub-add",url,"select",NULL};
  mpv_command(smPlayer,cmd);
}
void sm_mpv_speed(int percent) {
  if(!smPlayer)return;
  double rate=percent>0?percent/100.0:1.0;
  if(rate<0.25)rate=0.25;
  if(rate>4.0)rate=4.0;
  mpv_set_property(smPlayer,"speed",MPV_FORMAT_DOUBLE,&rate);
}
static void smTracks(VideoSimulacao *s) {
  int count=(int)smInt("track-list/count",0);
  s->nAudio=s->nLeg=0;
  for(int i=0;i<count && i<128;i++) {
    char prop[80]; int64_t id;
    snprintf(prop,sizeof(prop),"track-list/%d/type",i);
    char *kind=mpv_get_property_string(smPlayer,prop);
    int audio=kind && !strcmp(kind,"audio");
    int subtitle=kind && !strcmp(kind,"sub");
    if(!audio&&!subtitle){if(kind)mpv_free(kind);continue;}
    int slot=audio?s->nAudio:s->nLeg;
    if(slot>=8){mpv_free(kind);continue;}
    VideoFaixa *f=audio?&s->audio[slot]:&s->leg[slot];
    memset(f,0,sizeof(*f));
    snprintf(prop,sizeof(prop),"track-list/%d/id",i);
    id=smInt(prop,0);
    f->numero=(int)id;f->stream_id=(int)id;f->stream_index=-1;f->ordinalMkv=-1;
    snprintf(prop,sizeof(prop),"track-list/%d/lang",i);
    char *language=mpv_get_property_string(smPlayer,prop);
    if(language){snprintf(f->idioma,sizeof(f->idioma),"%s",language);mpv_free(language);}
    snprintf(prop,sizeof(prop),"track-list/%d/title",i);
    char *title=mpv_get_property_string(smPlayer,prop);
    if(title){snprintf(f->rotulo,sizeof(f->rotulo),"%s",title);snprintf(f->nome,sizeof(f->nome),"%s",title);mpv_free(title);}
    else snprintf(f->rotulo,sizeof(f->rotulo),"%s %d",audio?"Audio":"Legenda",slot+1);
    if(audio){smAudioId[slot]=(int)id;s->nAudio++;}
    else {smSubId[slot]=(int)id;s->nLeg++;}
    mpv_free(kind);
  }
  int aid=(int)smInt("aid",-1),sid=(int)smInt("sid",-1);
  for(int i=0;i<s->nAudio;i++)if(smAudioId[i]==aid)smSelectedAudio=i;
  for(int i=0;i<s->nLeg;i++)if(smSubId[i]==sid)smSelectedSub=i;
  s->audioAtual=smSelectedAudio;s->legAtual=smSelectedSub;
}
void sm_mpv_pump(VideoSimulacao *s) {
  if(!smPlayer || !s)return;
  mpv_event *ev;
  while((ev=mpv_wait_event(smPlayer,0))->event_id!=MPV_EVENT_NONE) {
    if(ev->event_id==MPV_EVENT_FILE_LOADED) {smReady=1;smFailed=0;smEnded=0;}
    if(ev->event_id==MPV_EVENT_END_FILE && smPlaying) {
      mpv_event_end_file *end=(mpv_event_end_file *)ev->data;
      if(end && end->reason==MPV_END_FILE_REASON_ERROR) {
        smFailed=1;smReady=0;
        snprintf(smError,sizeof(smError),"Falha ao decodificar ou carregar a fonte");
      } else if(end && end->reason==MPV_END_FILE_REASON_EOF)smEnded=1;
    }
  }
  s->aceita=smPlaying;s->tocando=smReady&&!smEnded&&!smFailed;
  s->iniciando=smPlaying&&!smReady&&!smFailed;
  s->pronto=smReady&&!smFailed;
  s->pos=smDouble("time-pos");s->duracao=smDouble("duration");
  s->bufferFim=s->pos+smDouble("demuxer-cache-duration");
  s->largura=(int)smInt("video-params/w",0);
  s->altura=(int)smInt("video-params/h",0);
  smPaused=smInt("pause",smPaused)!=0; // enum? MPV_FORMAT_FLAG checked separately below
  int paused=smPaused;
  if(mpv_get_property(smPlayer,"pause",MPV_FORMAT_FLAG,&paused)>=0)smPaused=paused;
  s->bufferandoMs=0;s->reconectando=0;
  if(smReady)smTracks(s);
}
int sm_mpv_failed(void){return smFailed;}
int sm_mpv_ended(void){return smEnded;}
const char *sm_mpv_error(void){return smError;}

static GLuint smShader(GLenum kind,const char *text) {
  GLuint shader=glCreateShader(kind);
  GLint good=0;
  glShaderSource(shader,1,&text,NULL);
  glCompileShader(shader);
  glGetShaderiv(shader,GL_COMPILE_STATUS,&good);
  if(!good){glDeleteShader(shader);return 0;}
  return shader;
}
static int smCompositor(void) {
  if(smProgram)return 1;
  const char *vert="attribute vec2 aPos; attribute vec2 aUV; varying vec2 vUV;"
                   "void main(){gl_Position=vec4(aPos,0.0,1.0);vUV=aUV;}";
  const char *frag="precision mediump float;varying vec2 vUV;uniform sampler2D uTex;"
                   "void main(){gl_FragColor=texture2D(uTex,vUV);}";
  GLuint v=smShader(GL_VERTEX_SHADER,vert),f=smShader(GL_FRAGMENT_SHADER,frag);
  GLint linked=0;
  if(!v||!f){if(v)glDeleteShader(v);if(f)glDeleteShader(f);return 0;}
  smProgram=glCreateProgram();
  glAttachShader(smProgram,v);glAttachShader(smProgram,f);
  glBindAttribLocation(smProgram,0,"aPos");glBindAttribLocation(smProgram,1,"aUV");
  glLinkProgram(smProgram);glGetProgramiv(smProgram,GL_LINK_STATUS,&linked);
  glDeleteShader(v);glDeleteShader(f);
  if(!linked){glDeleteProgram(smProgram);smProgram=0;return 0;}
  return 1;
}
static int smTarget(int w,int h) {
  if(w<=0||h<=0)return 0;
  if(smFbo && smTexture && w==smTexW && h==smTexH)return 1;
  if(smFbo){glDeleteFramebuffers(1,&smFbo);smFbo=0;}
  if(smTexture){glDeleteTextures(1,&smTexture);smTexture=0;}
  glGenTextures(1,&smTexture);glBindTexture(GL_TEXTURE_2D,smTexture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glGenFramebuffers(1,&smFbo);
  glBindFramebuffer(GL_FRAMEBUFFER,smFbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,smTexture,0);
  int ok=glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  if(!ok)return 0;
  smTexW=w;smTexH=h;
  return 1;
}
void sm_mpv_composite(void) {
  if(!smRenderer||!smPlaying||!smReady||smFailed||smW<1||smH<1)return;
  // A UI ja foi desenhada. O framebuffer possui alfa zero nos furos do video.
  // Desenhar o video apenas sob estes pixels preserva player OSD, legendas e modais.
  int x=smX*smDisplayW/1920, y=smY*smDisplayH/1080;
  int w=smW*smDisplayW/1920, h=smH*smDisplayH/1080;
  if(w<2||h<2||!smCompositor()||!smTarget(w,h))return;
  mpv_opengl_fbo target={(int)smFbo,w,h,0};
  int flip=0;
  mpv_render_param args[]={{MPV_RENDER_PARAM_OPENGL_FBO,&target},
                            {MPV_RENDER_PARAM_FLIP_Y,&flip},
                            {MPV_RENDER_PARAM_INVALID,NULL}};
  GLint prevProgram=0;
  glGetIntegerv(GL_CURRENT_PROGRAM,&prevProgram);
  mpv_render_context_render(smRenderer,args);
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  glViewport(0,0,smDisplayW,smDisplayH);
  float x0=-1.0f+2.0f*x/smDisplayW,x1=-1.0f+2.0f*(x+w)/smDisplayW;
  float y0=1.0f-2.0f*y/smDisplayH,y1=1.0f-2.0f*(y+h)/smDisplayH;
  const GLfloat v[]={
    x0,y1,0,0, x1,y1,1,0, x0,y0,0,1, x1,y0,1,1
  };
  glUseProgram(smProgram);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA,GL_ONE,GL_ONE_MINUS_DST_ALPHA,GL_ONE);
  glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,smTexture);
  glUniform1i(glGetUniformLocation(smProgram,"uTex"),0);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);
  glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(GLfloat),v);
  glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(GLfloat),v+2);
  glDrawArrays(GL_TRIANGLE_STRIP,0,4);
  glDisableVertexAttribArray(0);glDisableVertexAttribArray(1);
  glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram((GLuint)prevProgram);
}
void sm_mpv_report_swap(void) {
  if(smRenderer&&smReady)mpv_render_context_report_swap(smRenderer);
}
#endif
