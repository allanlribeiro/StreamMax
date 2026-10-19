#include "../src/streammax_source_audio.h"
#include <assert.h>
#include <stdio.h>

static void sim(const char *arq, const char *name, const char *title, const char *desc) {
  assert(streammax_audio_dual_fonte(arq,name,title,desc));
}
static void nao(const char *arq, const char *name, const char *title, const char *desc) {
  assert(!streammax_audio_dual_fonte(arq,name,title,desc));
}
int main(void) {
  // Releases reais usam ponto, hifen, underscore, espaco e token unido.
  sim("Filme.2160p.DUAL.AUDIO.TB.mkv","","","");
  sim("Filme.1080p.DUAL-AUDIO.mkv","","","");
  sim("Filme.1080p.DUAL_AUDIO.mkv","","","");
  sim("Filme.1080p.DUALAUDIO.mkv","","","");
  sim("Filme.1080p.DUAL.mkv","","","");
  sim("","","Homem Aranha | Dual Audio | 25.9 GB","");
  sim("","Torrentio TB","","Dual Audio / 25.9 GB");
  sim("","Torrentio TB","", "Áudio Dual | WEB-DL");
  nao("Filme.DUALITY.2026.mkv","","","");
  nao("Filme.mkv","","Dual 2026","");
  nao("","","","Subs: pt-BR");
  nao("Filme.PT-BR.SUBS.mkv","","","");
  nao("Filme.Brazilian.Portuguese.mkv","","","");
  nao("Filme.mkv","","","Dolby Audio Atmos");
  nao(NULL,NULL,NULL,NULL);
  // O release pode listar as DUAS trilhas de fato, em nome, title ou
  // descricao; so nesse caso o parser pode mostrar duas bandeiras.
  assert(streammax_audio_dual_idiomas_fonte(
      "Filme.DUAL.AUDIO.PTBR.ENG.mkv", "", "", "") == (SM_DUAL_BR | SM_DUAL_EN));
  assert(streammax_audio_dual_idiomas_fonte(
      "", "", "Torrentio TB / Dual Audio / 🇧🇷 🇺🇸", "") == (SM_DUAL_BR | SM_DUAL_EN));
  assert(streammax_audio_dual_idiomas_fonte(
      "", "", "", "Dual Audio\nAudio: English\nSubs: pt-BR") == SM_DUAL_EN);
  assert(streammax_audio_dual_idiomas_fonte(
      "", "", "", "Dual Audio 🇧🇷\nLegendas: English") == SM_DUAL_BR);
  // PT-PT e portugues generico sao faixas portuguesas, mas nunca
  // devem virar pt-BR na filtragem; a bandeira BR e apenas visual.
  assert(streammax_audio_dual_idiomas_fonte(
      "Filme.DUAL.AUDIO.PT-PT.EN-GB.mkv", "", "", "") ==
      (SM_DUAL_PTPT | SM_DUAL_EN));
  assert(streammax_audio_dual_idiomas_fonte(
      "", "", "Dual Audio", "Audio: Portuguese / English") ==
      (SM_DUAL_PT | SM_DUAL_EN));
  assert(streammax_audio_dual_idiomas_fonte(
      "", "", "Dual Audio", "Subs: Portuguese / English") == 0);
  // A tag Dual Audio, sozinha, NAO permite deduzir PT-BR nem English.
  assert(streammax_audio_dual_idiomas_fonte(
      "", "Torrentio TB", "Dual Audio", "25.9 GB") == 0);
  assert(streammax_audio_dual_idiomas_fonte(
      "", "", "Dual Audio / Subs: PT-BR / English", "") == 0);
  assert(streammax_audio_dual_idiomas_fonte(
      "", "", "Dual Audio", "Subs: 🇧🇷 / 🇺🇸") == 0);
  assert(streammax_audio_dual_idiomas_fonte(
      "", "", "Audio: pt-BR English", "") == 0);
  assert(streammax_audio_dual_idiomas_fonte(
      "", "", "Dual Audio / 🇺🇸", "") == SM_DUAL_EN);
  puts("OK: Dual Audio e idiomas com evidencias reais, sem transformar legenda em audio");
  return 0;
}
