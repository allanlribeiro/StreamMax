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
  puts("OK: Dual Audio marcado apenas por indicacao explicita da fonte");
  return 0;
}
