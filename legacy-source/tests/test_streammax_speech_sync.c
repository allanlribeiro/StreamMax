#include "legfala.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
static LegendaDocumentoInfo info = {.sessao=7,.flags=LEGENDA_DOC_COMPLETO,.identidade="fixture"};
static float measure(const char *s,void *u){(void)u;return (float)strlen(s)*13;}
static void check_lines(const char *s,float width) {
  char copy[1536];snprintf(copy,sizeof copy,"%s",s);char *save=NULL;
  for(char *line=strtok_r(copy,"\n",&save);line;line=strtok_r(NULL,"\n",&save)) {
    assert(strlen(line)<=400 && measure(line,NULL)<=width);
    for(const unsigned char *p=(const unsigned char *)line;*p;) {
      int n=*p<0x80?1:(*p&0xe0)==0xc0?2:(*p&0xf0)==0xe0?3:(*p&0xf8)==0xf0?4:0;
      assert(n);p++;
      for(int i=1;i<n;i++,p++)assert((*p&0xc0)==0x80);
    }
  }
}
static void check_contents(const char *source,const char *wrapped) {
  // Wrapping may replace whitespace, but must preserve every visible byte.
  for(;;) {
    while(*source && isspace((unsigned char)*source))source++;
    while(*wrapped && isspace((unsigned char)*wrapped))wrapped++;
    assert(*source==*wrapped);
    if(!*source)break;
    source++;wrapped++;
  }
}
static void check(const char *body) {
  LegendaDocumento *d=legenda_documento_criar(body,&info);
  LegFala f={0}; assert(d && legfala_abrir(&f,d,42));
  assert(f.n==3 && !f.capturada);
  int offset=0;
  assert(!legfala_calcular(&f,&offset));
  assert(!legfala_capturar(&f,NAN,1000));
  assert(!legfala_capturar(&f,-1,1000));
  assert(legfala_capturar(&f,12300,11000) && f.foco==1);
  assert(legfala_calcular(&f,&offset) && offset==2000);
  LegendaCue cue;
  assert(legenda_documento_cues(d,12,-offset,&cue,1)==1 && cue.inicio==10);
  assert(legfala_capturar(&f,8300,10000));
  assert(legfala_calcular(&f,&offset) && offset==-2000);
  assert(legenda_documento_cues(d,8,-offset,&cue,1)==1 && cue.inicio==10);
  assert(legfala_capturar(&f,10300,10000));
  assert(legfala_calcular(&f,&offset) && offset==0);
  assert(legfala_capturar(&f,70301,10000));
  assert(!legfala_calcular(&f,&offset)); // no silent clamp
  assert(legfala_capturar(&f,70300,10000));
  assert(legfala_calcular(&f,&offset) && offset==60000);
  assert(legfala_capturar(&f,12500,11500) && f.foco==1); // nearest tie keeps earlier
  legfala_mover(&f,INT_MIN); assert(f.foco==0);
  assert(!legfala_mover(&f,-1));
  legfala_mover(&f,INT_MAX); assert(f.foco==2);
  assert(!legfala_cue(&f,3));
  legfala_fechar(&f); assert(!f.doc && !f.indices && !f.n);
  legenda_documento_liberar(d);
}
int main(void) {
  check("1\n00:00:01,000 --> 00:00:02,000\nPrimeira fala\n\n2\n00:00:10,000 --> 00:00:12,000\nFala completa\ncom duas linhas.\n\n3\n00:00:13,000 --> 00:00:14,000\nTerceira fala\n");
  check("WEBVTT\n\n00:01.000 --> 00:02.000\nPrimeira fala\n\n00:10.000 --> 00:12.000\nFala completa\ncom duas linhas.\n\n00:13.000 --> 00:14.000\nTerceira fala\n");
  check("[Events]\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\nDialogue: 0,0:00:01.00,0:00:02.00,Default,,0,0,0,,Primeira fala\nDialogue: 0,0:00:10.00,0:00:12.00,Default,,0,0,0,,{\\b1}Fala completa\\Ncom duas linhas.\nDialogue: 0,0:00:13.00,0:00:14.00,Default,,0,0,0,,Terceira fala\n");
  LegendaCue c={.inicio=1,.fim=2}; char text[768];
  strcpy(c.texto,"{\\an8}Português completo\\Ncom acentuação.");
  legfala_frase(&c,text,sizeof text);assert(!strcmp(text,"Português completo\ncom acentuação."));
  char tiny[3]; strcpy(c.texto,"éé");legfala_frase(&c,tiny,sizeof tiny);assert(!strcmp(tiny,"é"));
  char tiny2[2];legfala_frase(&c,tiny2,sizeof tiny2);assert(!tiny2[0]);
  memset(c.texto,'A',700);strcpy(c.texto+700," fim da frase");
  LegendaDocumento *d=legenda_documento_de_cues(&c,1,&info);LegFala f={0};
  assert(legfala_abrir(&f,d,3));legfala_frase(legfala_cue(&f,0),text,sizeof text);
  assert(!strcmp(text,c.texto) && strstr(text,"fim da frase"));
  assert(legfala_abrir(&f,f.doc,4) && f.n==1 && f.geracao==4);
  legfala_fechar(&f);legenda_documento_liberar(d);
  LegendaDocumentoInfo incomplete=info;incomplete.flags=0;
  d=legenda_documento_de_cues(&c,1,&incomplete);assert(!legfala_abrir(&f,d,1));legenda_documento_liberar(d);
  incomplete.flags=LEGENDA_DOC_COMPLETO|LEGENDA_DOC_JANELA;
  d=legenda_documento_de_cues(&c,1,&incomplete);assert(!legfala_abrir(&f,d,1));legenda_documento_liberar(d);
  assert(!legfala_abrir(&f,NULL,1));assert(!legfala_abrir(NULL,NULL,0));
  char original[768],wrapped[1536];memset(original,'X',700);strcpy(original+700," fim completo");
  assert(legfala_quebrar(original,300,wrapped,sizeof wrapped,measure,NULL));
  int xs=0;for(char *p=wrapped;*p;p++)xs+=*p=='X';
  assert(xs==700 && strstr(wrapped,"completo"));
  check_lines(wrapped,300);check_contents(original,wrapped);
  assert(legfala_quebrar(original,100000,wrapped,sizeof wrapped,measure,NULL));
  check_lines(wrapped,100000);check_contents(original,wrapped);
  assert(strchr(wrapped,'\n')); // native 512-byte token limit even on very wide screens
  assert(legfala_quebrar("Português completo com acentuação.",300,wrapped,sizeof wrapped,measure,NULL));
  assert(strstr(wrapped,"acentuação."));
  const char *examples[]={"  Frase com\tpalavras e acentos: coração.\n\nSegunda linha!  ","éééééééééééééééééééééééé", "🙂🙂🙂🙂🙂🙂漢字漢字漢字"};
  for(size_t i=0;i<sizeof examples/sizeof *examples;i++) {
    assert(legfala_quebrar(examples[i],65,wrapped,sizeof wrapped,measure,NULL));
    check_lines(wrapped,65);check_contents(examples[i],wrapped);
  }
  memset(original,'W',767);original[767]=0;
  assert(legfala_quebrar(original,13,wrapped,sizeof wrapped,measure,NULL));
  check_lines(wrapped,13);check_contents(original,wrapped);
  assert(!legfala_quebrar(original,0,wrapped,sizeof wrapped,measure,NULL));
  assert(!legfala_quebrar(original,300,wrapped,2,measure,NULL) && !wrapped[0]);
  assert(!legfala_quebrar("W",12,wrapped,sizeof wrapped,measure,NULL) && !wrapped[0]);
  assert(!legfala_quebrar("W",NAN,wrapped,sizeof wrapped,measure,NULL));
  assert(legfala_quebrar("",300,wrapped,sizeof wrapped,measure,NULL) && !wrapped[0]);
  puts("OK: parser real SRT/VTT/ASS, frases completas, UTF-8, offsets e limites");
}
