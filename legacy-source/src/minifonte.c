#include "minifonte.h"
#include "js.h"
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static int seguro(const char *s) {
  if (!s || strlen(s) >= MINI_URL) return 0;
  for (; *s; s++) if ((unsigned char)*s <= 32 || (unsigned char)*s == 127 || *s == '\\') return 0;
  return 1;
}
static const char *autoridade(const char *s, char *origem, size_t tam) {
  int https = !strncasecmp(s, "https://", 8), http = !strncasecmp(s, "http://", 7);
  if (!https && !http) return NULL;
  const char *a = s + (https ? 8 : 7), *f = a + strcspn(a, "/?#");
  if (f == a || memchr(a, '@', (size_t)(f-a)) || (size_t)(f-s) >= tam) return NULL;
  size_t n = (size_t)(f-s);
  memcpy(origem, s, n); origem[n] = 0;
  for (size_t i = 0; i < n; i++) origem[i] = (char)tolower((unsigned char)origem[i]);
  const char *port = NULL;
  if (*a == '[') { const char *b = memchr(a, ']', (size_t)(f-a)); if (!b) return NULL; if (b+1 < f && b[1] == ':') port = b+1; }
  else port = memchr(a, ':', (size_t)(f-a));
  if (port) {
    if (port+1 == f) return NULL;
    for (const char *p = port+1; p < f; p++) if (!isdigit((unsigned char)*p)) return NULL;
    long p = strtol(port+1, NULL, 10); if (p <= 0 || p > 65535) return NULL;
    if ((https && p == 443) || (http && p == 80)) origem[port-s] = 0;
  }
  return f;
}
int minifonte_mesma_origem(const char *a, const char *b) {
  char aa[MINI_URL], bb[MINI_URL];
  return a && b && seguro(a) && seguro(b) && autoridade(a, aa, sizeof aa) && autoridade(b, bb, sizeof bb) && !strcmp(aa, bb);
}
int minifonte_url(const char *base, const char *ref, char *dst, size_t tam) {
  char joined[MINI_URL * 2], origin[MINI_URL], path[MINI_URL * 2];
  const char *b, *p; size_t n;
  if (!dst || !tam) return 0;
  dst[0] = 0;
  if (!base || !seguro(base) || !seguro(ref) || !(b = autoridade(base, origin, sizeof origin))) return 0;
  if (!strncasecmp(ref, "https://", 8) || !strncasecmp(ref, "http://", 7)) {
    if (snprintf(joined, sizeof joined, "%s", ref) >= (int)sizeof joined) return 0;
  } else if (!strncmp(ref, "//", 2)) {
    if (snprintf(joined, sizeof joined, "%.*s%s", (int)(strchr(base, ':') - base + 1), base, ref) >= (int)sizeof joined) return 0;
  } else {
    // A colon before the first path separator denotes an unsupported URI scheme.
    const char *colon = strchr(ref, ':'); if (colon && (size_t)(colon-ref) < strcspn(ref, "/?#")) return 0;
    size_t root = (size_t)(b-base), end = root + strcspn(b, "?#");
    if (*ref == '/') n = root;
    else if (*ref == '?' || !*ref || *ref == '#') n = end;
    else { n = end; while (n > root && base[n-1] != '/') n--; }
    int slash = n == root && *ref != '/';
    if (snprintf(joined, sizeof joined, "%.*s%s%s", (int)n, base, slash ? "/" : "", ref) >= (int)sizeof joined) return 0;
  }
  if (!(p = autoridade(joined, origin, sizeof origin))) return 0;
  const char *end = p + strcspn(p, "?#"), *query = *end == '?' ? end : NULL;
  size_t used = 0;
  if (p == end) path[used++] = '/';
  while (p < end) {
    const char *seg = p, *next;
    if (*p == '/') p++;
    next = p; while (next < end && *next != '/') next++;
    size_t k = (size_t)(next-p);
    if (k == 1 && p[0] == '.') { if (next == end && (!used || path[used-1] != '/')) path[used++] = '/'; }
    else if (k == 2 && p[0] == '.' && p[1] == '.') {
      while (used && path[used-1] != '/') used--;
      if (used) used--;
      if (next == end) path[used++] = '/';
    } else {
      size_t k2 = (size_t)(next-seg);
      if (used + k2 >= sizeof path) return 0;
      memcpy(path+used, seg, k2); used += k2;
    }
    p = next;
  }
  if (!used) path[used++] = '/';
  path[used] = 0;
  size_t qlen = query ? strcspn(query, "#") : 0;
  int count = snprintf(dst, tam, "%s%s%.*s", origin, path, (int)qlen, query ? query : "");
  if (count < 0 || (size_t)count >= tam || !seguro(dst)) { dst[0] = 0; return 0; }
  return 1;
}
int minifonte_e_hls(const char *url) {
  if (!url) return 0;
  size_t n = strcspn(url, "?#"); return n >= 5 && !strncasecmp(url+n-5, ".m3u8", 5);
}
static const char *objeto(const char *obj, const char *fim, const char *name) {
  if (!obj || !fim || obj >= fim || *obj != '{') return NULL;
  const char *p = obj+1;
  while (p < fim) {
    while (p < fim && (isspace((unsigned char)*p) || *p==',')) p++;
    if (p >= fim || *p != '"') return NULL;
    char key[96] = ""; const char *k=p++;
    while (p < fim && *p!='"') { if (*p=='\\' && p+1<fim) p++; p++; }
    if (p++ >= fim) return NULL;
    int match = (size_t)(p-k)<sizeof key && js_cadeia(k,key,sizeof key) && !strcmp(key,name);
    while(p<fim && isspace((unsigned char)*p))p++;
    if(p>=fim || *p++!=':')return NULL;
    while(p<fim && isspace((unsigned char)*p))p++;
    if(p>=fim)return NULL;
    if(*p=='{' || *p=='[') { const char *end=js_fim(p);if(!end||end>fim)return NULL;if(match&&*p=='{')return p;p=end; }
    else if(*p=='"') {p++;while(p<fim&&*p!='"'){if(*p=='\\'&&p+1<fim)p++;p++;}if(p<fim)p++;}
    else while(p<fim&&*p!=','&&*p!='}')p++;
  }
  return NULL;
}
int minifonte_metadados(const char *obj, const char *fim, char *dst, size_t tam) {
  char hint[MINI_URL] = ""; dst[0] = 0;
  const char *objects[2] = {obj, objeto(obj, fim, "behaviorHints")};
  for (int i=0;i<2;i++) if (objects[i]) {
    hint[0] = 0;
    const char *f = js_fim(objects[i]);
    if (!f || f > fim) continue;
    if (!js_texto_raiz_em(objects[i], f, "thumbnailVtt", hint, sizeof hint)) {
      const char *t = objeto(objects[i], f, "thumbnails");
      char type[32] = "";
      if (t) {
        const char *tf = js_fim(t); if (!tf || tf > f) continue;
        js_texto_raiz_em(t, tf, "type", type, sizeof type);
        if (strcmp(type, "vtt") && strcmp(type, "text/vtt")) continue;
        js_texto_raiz_em(t, tf, "url", hint, sizeof hint);
      }
    }
    if (hint[0] && seguro(hint) && strlen(hint)+1 < tam) { memcpy(dst, hint, strlen(hint)+1); return 1; }
  }
  return 0;
}
static const char *linha(const char *p, char *dst, size_t tam) {
  const char *end = strchr(p, '\n'); if (!end) end = p+strlen(p);
  size_t n = (size_t)(end-p); if (n && p[n-1] == '\r') n--;
  if (n >= tam) return NULL;
  memcpy(dst,p,n);dst[n]=0; return *end ? end+1 : end;
}
static int folha(SeekrVtt *v, const char *url) {
  for (int i=v->nFolhas-1;i>=0;i--) if (!strcmp(v->folhas[i],url)) return i;
  if (v->nFolhas >= MINI_FOLHAS) return -1;
  char **p = realloc(v->folhas,(size_t)(v->nFolhas+1)*sizeof *p); if (!p) return -1;
  v->folhas=p;p[v->nFolhas]=malloc(strlen(url)+1);if (!p[v->nFolhas]) return -1;
  strcpy(p[v->nFolhas],url);return v->nFolhas++;
}
static int cue(SeekrVtt *v, long start, long end, const char *url, int x,int y,int w,int h) {
  if (start<0 || end<=start || end>86400000L || v->nCues>=MINI_CUES || x<0 || y<0 || w<0 || h<0 ||
      x>8192 || y>8192 || w>8192 || h>8192 || (w==0)!=(h==0) ||
      (v->nCues && start<v->cues[v->nCues-1].fim)) return 0;
  int f=folha(v,url);if(f<0)return 0;
  SeekrCue *p=realloc(v->cues,(size_t)(v->nCues+1)*sizeof *p);if(!p)return 0;
  v->cues=p;p[v->nCues++]=(SeekrCue){start,end,f,x,y,w,h};return 1;
}
static long tempo(const char *p, const char **fim) {
  long long parts[3]={0};int n=0;
  while (n<3) {
    if (!isdigit((unsigned char)*p)) return -1;
    while(isdigit((unsigned char)*p)){parts[n]=parts[n]*10+(*p++-'0');if(parts[n]>86400)return -1;}
    n++;if(*p!=':')break;p++;
  }
  if(n<2 || *p!='.' || parts[n-1]>=60 || (n==3 && parts[1]>=60))return -1;
  p++; int ms=0;for(int i=0;i<3;i++){if(!isdigit((unsigned char)*p))return -1;ms=ms*10+(*p++-'0');}
  if(*p && *p!=' ' && *p!='\t')return -1;
  long long value=(n==3?parts[0]*3600000LL+parts[1]*60000LL:parts[0]*60000LL)+parts[n-1]*1000+ms;
  if(value>86400000)return -1;
  if(fim)*fim=p;
  return (long)value;
}
static int inteiros(const char *s, int *values, int n, char separator) {
  for (int i=0;i<n;i++) {
    if(!isdigit((unsigned char)*s))return 0;
    char *end;long v=strtol(s,&end,10);if(v>8192)return 0;values[i]=(int)v;
    if(i+1<n){if(*end!=separator)return 0;s=end+1;}else if(*end)return 0;
  }
  return 1;
}
int minifonte_vtt(const char *text,const char *base,SeekrVtt *v) {
  char line[MINI_URL+128], url[MINI_URL]; const char *p=text; memset(v,0,sizeof *v);
  if(!p || strlen(p)>MINI_DOCUMENTO)return 0;
  if(!strncmp(p,"\xef\xbb\xbf",3))p+=3;
  if(strncmp(p,"WEBVTT",6) || (p[6] && !isspace((unsigned char)p[6])))return 0;
  while(*p) {
    p=linha(p,line,sizeof line);if(!p)goto fail;
    if(!strncmp(line,"NOTE",4) || !strcmp(line,"STYLE") || !strcmp(line,"REGION")){
      do{p=linha(p,line,sizeof line);if(!p)goto fail;}while(*p && line[0]);continue;
    }
    const char *arrow=strstr(line,"-->");if(!arrow)continue;
    const char *f;long start=tempo(line,&f);if(start<0 || f>arrow)goto fail;
    const char *q=arrow+3;while(*q==' '||*q=='\t')q++;
    long end=tempo(q,NULL);if(end<=start)goto fail;
    p=linha(p,line,sizeof line);if(!p || !line[0])goto fail;
    int x=0,y=0,w=0,h=0;char *hash=strchr(line,'#');
    if(hash){if(hash==line)goto fail;*hash++=0;if(strncmp(hash,"xywh=",5))goto fail;hash+=5;if(!strncmp(hash,"pixel:",6))hash+=6;
      int vals[4];if(!inteiros(hash,vals,4,',')||vals[2]<=0||vals[3]<=0)goto fail;
      x=vals[0];y=vals[1];w=vals[2];h=vals[3];}
    if(!minifonte_url(base,line,url,sizeof url) || !cue(v,start,end,url,x,y,w,h))goto fail;
  }
  return v->nCues;
fail: seekr_vtt_liberar(v);return 0;
}
static int atributo(const char *line,const char *key,char *out,size_t tam) {
  const char *p=strchr(line,':');if(!p)return 0;p++;
  while(*p){while(*p==','||isspace((unsigned char)*p))p++;const char *k=p;while(*p && *p!='=' && *p!=',')p++;
    if(*p!='=')return 0;
    size_t kn=(size_t)(p-k);p++;const char *a;size_t n;
    if(*p=='"'){a=++p;while(*p && *p!='"')p++;if(*p!='"')return 0;n=(size_t)(p-a);p++;}
    else{a=p;while(*p && *p!=',')p++;n=(size_t)(p-a);}
    if(strlen(key)==kn && !memcmp(k,key,kn)){if(n>=tam)return 0;memcpy(out,a,n);out[n]=0;return 1;}
    while(*p && *p!=',')p++;
  }return 0;
}
int minifonte_cue(const SeekrVtt *v,long pos) {
  if(!v||!v->nCues||pos<v->cues[0].ini)return -1;
  int lo=0,hi=v->nCues-1;
  while(lo<hi){int m=lo+(hi-lo+1)/2;if(v->cues[m].ini<=pos)lo=m;else hi=m-1;}
  return pos<v->cues[lo].fim?lo:-1;
}
static int res(const char *s,int *w,int *h){int v[2];if(!inteiros(s,v,2,'x')||!v[0]||!v[1])return 0;*w=v[0];*h=v[1];return 1;}
int minifonte_hls_indice(const char *text,const char *base,char *out,size_t tam,int *w,int *h) {
  char line[MINI_URL+128],uri[MINI_URL],rr[64],codec[64];const char *p=text;int best=INT_MAX;out[0]=0;*w=*h=0;
  if(!p||strncmp(p,"#EXTM3U",7)||strlen(p)>MINI_DOCUMENTO)return 0;
  while(*p){p=linha(p,line,sizeof line);if(!p)return 0;if(strncmp(line,"#EXT-X-IMAGE-STREAM-INF:",24))continue;
    int ww=0,hh=0;codec[0]=0;
    if(atributo(line,"CODECS",codec,sizeof codec) && strcmp(codec,"jpeg") && strcmp(codec,"png"))continue;
    if(atributo(line,"RESOLUTION",rr,sizeof rr) && !res(rr,&ww,&hh))continue;
    int pixels=ww&&hh?ww*hh:INT_MAX-1;if(pixels>=best)continue;
    char candidate[MINI_URL];
    if(!atributo(line,"URI",uri,sizeof uri)||!uri[0]||!minifonte_url(base,uri,candidate,sizeof candidate)||strlen(candidate)>=tam)continue;
    strcpy(out,candidate);
    best=pixels;*w=ww;*h=hh;
  }return out[0]!=0;
}
int minifonte_hls_imagens(const char *text,const char *base,int dw,int dh,SeekrVtt *v) {
  char line[MINI_URL+128],url[MINI_URL],val[128];const char *p=text;double start=0,duration=0,tile=0;int w=dw,h=dh,cols=1,rows=1,images=0,ended=0;
  memset(v,0,sizeof *v);if(!p||strncmp(p,"#EXTM3U",7)||strlen(p)>MINI_DOCUMENTO)return 0;
  while(*p){p=linha(p,line,sizeof line);if(!p)goto fail;
    if(!strcmp(line,"#EXT-X-IMAGES-ONLY"))images=1;
    else if(!strcmp(line,"#EXT-X-ENDLIST"))ended=1;
    else if(!strncmp(line,"#EXT-X-MEDIA-SEQUENCE:",22)){char *f;long n=strtol(line+22,&f,10);if(*f||n)goto fail;}
    else if(!strncmp(line,"#EXT-X-BYTERANGE:",17))goto fail;
    else if(!strncmp(line,"#EXT-X-KEY:",11)){if(!atributo(line,"METHOD",val,sizeof val)||strcmp(val,"NONE"))goto fail;}
    else if(!strncmp(line,"#EXTINF:",8)){char *f;duration=strtod(line+8,&f);if(!isfinite(duration)||duration<=0||duration>86400||(*f && *f!=','))goto fail;}
    else if(!strncmp(line,"#EXT-X-TILES:",13)){
      if(!atributo(line,"RESOLUTION",val,sizeof val)||!res(val,&w,&h)||!atributo(line,"LAYOUT",val,sizeof val)||!res(val,&cols,&rows)||
         cols>100||rows>100||(unsigned long long)w*h*cols*rows>MINI_PIXELS||!atributo(line,"DURATION",val,sizeof val))goto fail;
      char *f;tile=strtod(val,&f);if(*f||!isfinite(tile)||tile<=0||tile>86400)goto fail;
    } else if(line[0] && line[0]!='#') {
      if(!duration||!minifonte_url(base,line,url,sizeof url))goto fail;
      if(!tile)tile=duration;
      double frames=ceil(duration/tile-0.000001);if(!isfinite(frames)||frames<=0||frames>cols*rows)goto fail;
      int count=(int)frames;
      for(int k=0;k<count;k++){double end=start+fmin(duration,(k+1)*tile);
        if(!cue(v,(long)llround((start+k*tile)*1000),(long)llround(end*1000),url,(k%cols)*w,(k/cols)*h,w,h))goto fail;}
      start+=duration;duration=tile=0;w=dw;h=dh;cols=rows=1;
    }
  }
  if(!images||!ended||duration)goto fail;
  return v->nCues;
fail:seekr_vtt_liberar(v);return 0;
}

static unsigned be16(const unsigned char *b) { return (unsigned)b[0]*256u+b[1]; }
static unsigned be32(const unsigned char *b) { return (be16(b)<<16)|be16(b+2); }
static unsigned le24(const unsigned char *b) { return b[0]|((unsigned)b[1]<<8)|((unsigned)b[2]<<16); }
int minifonte_imagem_tamanho(const unsigned char *b,size_t n,int *w,int *h) {
  unsigned x=0,y=0;
  if(!b||n>MINI_IMAGEM||n<10)return 0;
  if(n>=24&&!memcmp(b,"\x89PNG\r\n\x1a\n",8)&&be32(b+8)==13&&!memcmp(b+12,"IHDR",4)) {
    x=be32(b+16);y=be32(b+20);
  } else if(b[0]==255&&b[1]==216) {
    size_t p=2;
    while(p+4<=n) {
      if(b[p++]!=255)return 0;
      while(p<n&&b[p]==255)p++;
      if(p>=n)return 0;
      unsigned marker=b[p++];if(marker==0xda||marker==0xd9)return 0;
      if(marker==0x01||(marker>=0xd0&&marker<=0xd7))continue;
      if(p+2>n)return 0;
      unsigned len=be16(b+p);if(len<2||len>n-p)return 0;
      if(marker>=0xc0&&marker<=0xcf&&marker!=0xc4&&marker!=0xc8&&marker!=0xcc) {
        if(len<8)return 0;
        y=be16(b+p+3);x=be16(b+p+5);break;
      }
      p+=len;
    }
  } else if(n>=25&&!memcmp(b,"RIFF",4)&&!memcmp(b+8,"WEBP",4)) {
    if(n>=30&&!memcmp(b+12,"VP8X",4)){x=1+le24(b+24);y=1+le24(b+27);if(b[20]&2)return 0;}
    else if(!memcmp(b+12,"VP8 ",4)&&n>=30&&!memcmp(b+23,"\x9d\x01\x2a",3)) {
      x=(b[26]|((unsigned)b[27]<<8))&0x3fff;y=(b[28]|((unsigned)b[29]<<8))&0x3fff;
    } else if(!memcmp(b+12,"VP8L",4)&&n>=25&&b[20]==0x2f) {
      unsigned a=le24(b+21)|((unsigned)b[24]<<24);x=1+(a&0x3fff);y=1+((a>>14)&0x3fff);
    }
  }
  if(!x||!y||x>8192||y>8192||(unsigned long long)x*y>MINI_PIXELS)return 0;
  *w=(int)x;*h=(int)y;return 1;
}
