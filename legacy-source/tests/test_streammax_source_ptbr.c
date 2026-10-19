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
  // Marcas maiores que os dados nao podem ler alem do fim da string.
  nao("", "", "");
  nao("p", "pt", "D");
  nao("filme.pt", "", "Audio: p");
  sim("pt-br", "", "");
  nao(NULL, NULL, NULL);
  assert(streammax_dual_marca_release("Filme.2160p.DUAL.mkv"));
  assert(streammax_dual_marca_release("Filme.Dual.Audio.WEB-DL"));
  assert(streammax_dual_marca_release("Filme.DUAL.AUDIO.SUBS.mkv"));
  assert(streammax_dual_marca_release("Filme.DUAL_AUDIO.Legendas.mkv"));
  assert(streammax_dual_marca_release("Filme.DUAL ÁUDIO + SUBS"));
  assert(!streammax_dual_marca_release("Filme.DUAL.SUBS.mkv"));
  assert(!streammax_dual_marca_release("Filme.DUAL.LEGENDAS.mkv"));
  assert(!streammax_dual_marca_release("Filme.DUBLADO.mkv"));
  assert(!streammax_dual_marca_release(NULL));
  assert(streammax_dual_descricao_audio("Audio: Dual Audio\nSubs: PT-BR"));
  assert(streammax_dual_descricao_audio("Legendas: PT-BR\nAudio: Dual.Audio"));
  assert(!streammax_dual_descricao_audio("Subtitles: Dual Audio"));
  assert(!streammax_dual_descricao_audio("Legendas: DUAL\nAudio: English"));
  assert(!streammax_dual_descricao_audio(NULL));
  printf("OK: cenarios de dublagem PT-BR e limites de string no seletor\n");
  return 0;
}
