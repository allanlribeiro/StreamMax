#include "../src/streammax_source_ptbr.h"
#include <assert.h>
#include <stdio.h>

static void sim(const char *arquivo, const char *rotulo, const char *desc) {
  assert(streammax_pt_audio_sem_declaracao(arquivo, rotulo, desc));
}
static void nao(const char *arquivo, const char *rotulo, const char *desc) {
  assert(!streammax_pt_audio_sem_declaracao(arquivo, rotulo, desc));
}
int main(void) {
  // Releases populares em portugues brasileiro, mesmo sem audioTracks.
  sim("Filme.2160p.DUBLADO.mkv", "", "");
  sim("Serie.S01E01.PT-BR.WEB-DL.mkv", "", "");
  sim("Filme.PTBR.HDR.mkv", "", "");
  sim("Filme.1080p.pob.mkv", "", "");
  sim("Filme.Brazilian.Portuguese.mkv", "", "");
  sim("Filme.mkv", "Filme Dublada", "");
  sim("Filme.mkv", "", "Audio: Português\nSubs: English");
  sim("Filme.mkv", "", "Audio: pt-br\nSubtitles: en");
  sim("Filme.mkv", "", "Language: Portuguese");
  sim("Filme.mkv", "", "Idioma: Portuguese Brazilian");
  sim("Filme.mkv", "", "Subs: English\nDublado");
  sim("Filme.mkv", "", "Audio: 🇧🇷");
  // Sem metadados de audio, legenda PT-BR nao conta como dublagem.
  nao("Filme.1080p.PT-BR.SUBS.mkv", "", "");
  nao("Filme.1080p.mkv", "PT-BR LEGENDADO", "");
  nao("Filme.mkv", "", "Subtitles: Portuguese (Brazil)");
  nao("Filme.mkv", "", "Legendas: pt-br");
  nao("Filme.mkv", "", "Audio: English\nSubs: pt-br");
  nao("Filme.mkv", "", "Audio: Dolby Atmos");
  nao("Filme.mkv", "", "Language: English");
  nao("Filme.mkv", "", "Titulo do Filme Portugal");
  nao("redublado123.mkv", "", "");
  nao("Filme.DUB.EN.mkv", "", "");
  nao(NULL, NULL, NULL);
  printf("OK: 23 cenarios de dublagem PT-BR no seletor de fontes\n");
  return 0;
}
