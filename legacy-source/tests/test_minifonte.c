#include "minifonte.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *base="https://media.example/catalog/master.m3u8?token=private";
static void url(const char *ref,const char *expected) {
  char out[MINI_URL];assert(minifonte_url(base,ref,out,sizeof out));assert(!strcmp(out,expected));
}
int main(void) {
  char out[MINI_URL];SeekrVtt v={0};int w,h;
  url("thumbs/frames.vtt","https://media.example/catalog/thumbs/frames.vtt");
  url("../thumbs/a.jpg?key=abc#xywh=0,0,320,180","https://media.example/thumbs/a.jpg?key=abc");
  url("/frames.vtt","https://media.example/frames.vtt");
  url("//cdn.example/a.jpg","https://cdn.example/a.jpg");
  url("?sig=two","https://media.example/catalog/master.m3u8?sig=two");
  assert(!minifonte_url(base,"file:///etc/passwd",out,sizeof out));
  assert(!minifonte_url(base,"https://user:password@cdn.example/a",out,sizeof out));
  assert(!minifonte_url(base,"x\r\nAuthorization: secret",out,sizeof out));
  assert(minifonte_mesma_origem(base,"https://MEDIA.example:443/a"));
  assert(!minifonte_mesma_origem(base,"https://media.example:444/a"));
  assert(!minifonte_mesma_origem(base,"http://media.example/a"));
  assert(minifonte_e_hls(base));assert(!minifonte_e_hls("https://example/a.mp4?file=.m3u8"));
  const char *json="{\"url\":\"movie.mkv\",\"thumbnail\":\"poster.jpg\",\"behaviorHints\":{\"thumbnailVtt\":\"thumbs.vtt\"}}";
  assert(minifonte_metadados(json,json+strlen(json),out,sizeof out));assert(!strcmp(out,"thumbs.vtt"));
  json="{\"thumbnails\":{\"type\":\"text/vtt\",\"url\":\"/frames.vtt\"}}";
  assert(minifonte_metadados(json,json+strlen(json),out,sizeof out));
  json="{\"thumbnail\":\"poster.jpg\",\"nested\":{\"thumbnailVtt\":\"wrong.vtt\"}}";
  assert(!minifonte_metadados(json,json+strlen(json),out,sizeof out));
  const char *vtt="WEBVTT\r\n\r\nNOTE publisher note\r\nignore this\r\n\r\n1\r\n00:00.000 --> 00:10.000\r\nsheet.jpg?sig=one#xywh=0,0,320,180\r\n\r\n2\r\n00:10.000 --> 00:20.000\r\nsheet.jpg?sig=one#xywh=320,0,320,180\r\n\r\n00:20.000 --> 00:30.000\r\nframe.png\r\n";
  assert(minifonte_vtt(vtt,"https://cdn.example/thumbs/frames.vtt",&v)==3);
  assert(v.nFolhas==2 && v.cues[1].x==320 && !strcmp(v.folhas[0],"https://cdn.example/thumbs/sheet.jpg?sig=one"));
  assert(minifonte_cue(&v,9999)==0 && minifonte_cue(&v,10000)==1 && minifonte_cue(&v,20000)==2);
  seekr_vtt_liberar(&v);
  assert(!minifonte_vtt("WEBVTT\n\n00:10.000 --> 00:09.000\na.jpg\n",base,&v));
  assert(!minifonte_vtt("WEBVTT\n\n00:00.000 --> 00:10.000\na.jpg#xywh=-1,0,1,1\n",base,&v));
  assert(!minifonte_vtt("WEBVTT\n\n00:00.000 --> 00:10.000\na.jpg#xywh=999999999999999999999,0,1,1\n",base,&v));
  assert(!minifonte_vtt("WEBVTT\n\n00:00.000 --> 00:10.000\nfile:///a.jpg\n",base,&v));
  assert(!minifonte_vtt("WEBVTT\n\n00:00.000 --> 00:10.000\na.jpg\n\n00:09.000 --> 00:20.000\nb.jpg\n",base,&v));
  const char *master="#EXTM3U\n#EXT-X-STREAM-INF:BANDWIDTH=1000\nvideo.m3u8\n#EXT-X-IMAGE-STREAM-INF:BANDWIDTH=20,CODECS=\"jpeg\",RESOLUTION=320x180,URI=\"thumbs/index.m3u8\"\n";
  assert(minifonte_hls_indice(master,base,out,sizeof out,&w,&h));assert(w==320&&h==180);assert(!strcmp(out,"https://media.example/catalog/thumbs/index.m3u8"));
  const char *images="#EXTM3U\n#EXT-X-IMAGES-ONLY\n#EXT-X-TILES:RESOLUTION=320x180,LAYOUT=3x1,DURATION=10.0\n#EXTINF:25,\nsheet.jpg\n#EXT-X-ENDLIST\n";
  assert(minifonte_hls_imagens(images,out,w,h,&v)==3);
  assert(v.cues[2].x==640 && v.cues[2].ini==20000 && v.cues[2].fim==25000);
  seekr_vtt_liberar(&v);
  assert(!minifonte_hls_imagens("#EXTM3U\n#EXT-X-IMAGES-ONLY\n#EXTINF:10,\na.jpg\n",base,320,180,&v));
  assert(!minifonte_hls_imagens("#EXTM3U\n#EXT-X-IMAGES-ONLY\n#EXT-X-MEDIA-SEQUENCE:12\n#EXTINF:10,\na.jpg\n#EXT-X-ENDLIST\n",base,320,180,&v));
  assert(!minifonte_hls_imagens("#EXTM3U\n#EXT-X-IMAGES-ONLY\n#EXT-X-KEY:METHOD=AES-128,URI=key\n#EXTINF:10,\na.jpg\n#EXT-X-ENDLIST\n",base,320,180,&v));
  assert(!minifonte_hls_imagens("#EXTM3U\n#EXT-X-IMAGES-ONLY\n#EXT-X-TILES:RESOLUTION=8192x8192,LAYOUT=100x100,DURATION=10\n#EXTINF:10,\na.jpg\n#EXT-X-ENDLIST\n",base,0,0,&v));
  unsigned char png[24]={137,80,78,71,13,10,26,10,0,0,0,13,'I','H','D','R',0,0,1,64,0,0,0,180};
  assert(minifonte_imagem_tamanho(png,sizeof png,&w,&h)&&w==320&&h==180);
  png[16]=1;assert(!minifonte_imagem_tamanho(png,sizeof png,&w,&h));
  unsigned char jpg[]={255,216,255,224,0,2,255,192,0,8,8,0,180,1,64,1};
  assert(minifonte_imagem_tamanho(jpg,sizeof jpg,&w,&h)&&w==320&&h==180);
  for(size_t n=0;n<sizeof jpg;n++)assert(!minifonte_imagem_tamanho(jpg,n,&w,&h));
  // Random/truncated publisher input must stay within parser bounds.
  unsigned state=7;char noise[192];
  for(int t=0;t<10000;t++){
    size_t len=(size_t)t%sizeof noise;
    for(size_t i=0;i<len;i++){state=state*1664525u+1013904223u;noise[i]=(char)(1+(state%126));}noise[len]=0;
    minifonte_vtt(noise,base,&v);seekr_vtt_liberar(&v);
    minifonte_hls_imagens(noise,base,0,0,&v);seekr_vtt_liberar(&v);
    minifonte_imagem_tamanho((unsigned char*)noise,len,&w,&h);
  }
  puts("OK: URL, metadados, VTT, HLS, limites de imagem e entradas truncadas");
}
