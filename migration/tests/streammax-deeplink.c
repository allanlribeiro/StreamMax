// Testes de regressão do protocolo próprio do StreamMax.
// Compilar sem Android: cc -Isrc -pthread src/deeplink.c migration/tests/streammax-deeplink.c -o /tmp/streammax-deeplink
#include "deeplink.h"
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "Falhou linha %d: %s\n", __LINE__, #x); failures++; } } while (0)

int main(void) {
    DeepLink d;
    CHECK(deeplink_parse("streammax://meta/movie/tt0111161", &d));
    CHECK(d.tipo == DL_TITULO && !strcmp(d.imdb, "tt0111161") && !d.serie);
    CHECK(deeplink_parse("streammax://play/series/tt0944947/2/5", &d));
    CHECK(d.tipo == DL_TOCAR && d.serie && d.temporada == 2 && d.episodio == 5);
    CHECK(deeplink_parse("streammax://search?q=ficcao+cientifica", &d));
    CHECK(d.tipo == DL_BUSCA && !strcmp(d.consulta, "ficcao cientifica"));
    CHECK(!deeplink_parse("streammax://play/movie/tt0111161/2/5", &d));
    CHECK(!deeplink_parse("streammax://meta/movie/../tt0111161", &d));
    CHECK(deeplink_parse("nuvio-legacy://meta/movie/tt0111161", &d));
    CHECK(deeplink_parse("stremio:///detail/movie/tt0111161", &d));
    deeplink_postar("streammax://meta/movie/tt0111161");
    CHECK(deeplink_pegar(&d) && d.tipo == DL_TITULO);
    CHECK(!deeplink_pegar(&d));
    puts(failures ? "streammax-deeplink: falhou" : "streammax-deeplink: ok");
    return failures != 0;
}
