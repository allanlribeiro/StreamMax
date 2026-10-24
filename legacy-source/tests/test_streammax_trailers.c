#include "trailerhero.h"
#include "trailerhls.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void geometria(void) {
  const int tamanhos[][2] = {{1920,1080},{1920,804},{1080,1080},{720,1280},{0,0}};
  for (unsigned i = 0; i < sizeof tamanhos / sizeof tamanhos[0]; i++) {
    float x,w,h,asp = i == 4 ? 16.0f/9.0f : (float)tamanhos[i][0]/tamanhos[i][1];
    sm_trailer_hero_area(1920,1228.8f,438,tamanhos[i][0],tamanhos[i][1],&x,&w,&h);
    assert(fabsf(x+w-1920) < 0.01f && x >= 0);
    assert(w <= 1228.81f && h <= 438.01f && h > 0);
    assert(fabsf(w/h-asp) < 0.001f);  // No stretch/crop even for square or portrait sources.
  }
  assert((1-SM_TRAILER_FADE_ESQUERDA)*SM_TRAILER_FADE_BASE > 0.72f);
}

static int reduzir(const char *m, int teto, char *dst, size_t cap) {
  FILE *f = tmpfile(), *g = tmpfile();
  int w = 0, ok;
  assert(f && g); fputs(m, f);
  ok = sm_trailer_hls_reduzir(f,g,teto,&w);
  rewind(g); dst[fread(dst,1,cap-1,g)] = 0;
  fclose(f); fclose(g);
  return ok ? w : 0;
}
#define A_EN "#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID=\"a\",LANGUAGE=\"en\",DEFAULT=YES,URI=\"https://test/audio-en.m3u8\"\n"
#define A_PT "#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID=\"a\",LANGUAGE=\"pt-PT\",DEFAULT=NO,URI=\"https://test/audio-pt.m3u8\"\n"
#define A_BR "#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID=\"a\",LANGUAGE=\"pt-BR\",DEFAULT=NO,URI=\"https://test/audio-br.m3u8\"\n"
#define A_AD "#EXT-X-MEDIA:TYPE=AUDIO,GROUP-ID=\"a\",LANGUAGE=\"pt-BR\",DEFAULT=NO,CHARACTERISTICS=\"public.accessibility.describes-video\",URI=\"https://test/audio-ad.m3u8\"\n"
#define S_BR "#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID=\"s\",LANGUAGE=\"pt-BR\",FORCED=NO,URI=\"https://test/sub-br.m3u8\"\n"
#define S_FORCED "#EXT-X-MEDIA:TYPE=SUBTITLES,GROUP-ID=\"s\",LANGUAGE=\"pt-BR\",FORCED=YES,URI=\"https://test/sub-forced.m3u8\"\n"
#define V720 "#EXT-X-STREAM-INF:BANDWIDTH=1000000,RESOLUTION=1280x720,CODECS=\"avc1.640028,mp4a.40.2\",AUDIO=\"a\",SUBTITLES=\"s\"\nhttps://test/video720.m3u8\n"
#define V1080 "#EXT-X-STREAM-INF:BANDWIDTH=2000000,RESOLUTION=1920x1080,CODECS=\"avc1.640028,mp4a.40.2\",AUDIO=\"a\",SUBTITLES=\"s\"\nhttps://test/video1080.m3u8\n"
#define V4K "#EXT-X-STREAM-INF:BANDWIDTH=10000000,RESOLUTION=3840x2160,CODECS=\"hvc1.2.4\",AUDIO=\"a\"\nhttps://test/video4k.m3u8\n"

int main(int argc, char **argv) {
  char dst[32768];
  geometria();
  assert(reduzir(A_EN A_PT A_AD A_BR S_FORCED S_BR V720 V1080 V4K,0,dst,sizeof dst)==1920);
  assert(strstr(dst,"audio-br") && !strstr(dst,"audio-en") && !strstr(dst,"audio-pt") && !strstr(dst,"audio-ad"));
  assert(strstr(dst,"sub-br") && !strstr(dst,"sub-forced"));
  assert(strstr(dst,"video1080") && !strstr(dst,"video720") && !strstr(dst,"video4k"));
  assert(reduzir(A_EN S_BR V1080,1080,dst,sizeof dst)==1920);
  assert(strstr(dst,"audio-en") && strstr(dst,"sub-br"));  // Subtitle fallback is retained.
  assert(reduzir(A_EN A_BR V720 V1080,720,dst,sizeof dst)==1280);
  assert(strstr(dst,"audio-br") && !strstr(dst,"SUBTITLES="));
  assert(reduzir(A_EN V1080,1080,dst,sizeof dst)==1920 && strstr(dst,"audio-en"));
  assert(!reduzir(V1080,1080,dst,sizeof dst));  // Missing audio group is rejected.
  assert(!reduzir("<html>erro</html>",1080,dst,sizeof dst));
  assert(!reduzir(A_EN V4K,0,dst,sizeof dst));
  assert(!reduzir(A_EN V1080,480,dst,sizeof dst));
  assert(reduzir(A_EN A_PT A_BR V1080,1080,dst,sizeof dst)==1920 && strstr(dst,"audio-br"));
  if (argc > 1) {
    FILE *f = fopen(argv[1],"r"), *g = tmpfile(); int w = 0;
    assert(f && g && sm_trailer_hls_reduzir(f,g,1080,&w) && w <= 1920);
    rewind(g); dst[fread(dst,1,sizeof dst-1,g)] = 0;
    assert(strstr(dst,"LANGUAGE=\"pt-BR\""));
    fclose(f); fclose(g);
    puts("OK: master real Apple BR reduzido com faixas portuguesas");
  }
  puts("OK: proporcao/ancoragem e prioridade de audio/legendas dos trailers");
  return 0;
}
