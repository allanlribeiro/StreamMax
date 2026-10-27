> Histórico da versão 1.0.19. O usuário cancelou o recurso após o teste na TV.
> A integração e o painel foram retirados do código na versão 1.0.20.

# Miniaturas ao avançar e voltar — StreamMax 1.0.19

O player Android mostra a imagem correspondente à posição escolhida na barra
de tempo quando a própria fonte publica uma sequência de miniaturas.
Não precisa de chave do Seekr nem de outro serviço de imagens.

Em **Ajustes → Reprodução → Miniatura ao avançar**, a opção
**Miniaturas na barra de tempo** vem ligada. **Fita de miniaturas** permite
mostrar também os quadros anterior e seguinte. A distância entre as imagens
depende do material publicado pelo provedor.

## Fontes compatíveis

- WebVTT de miniaturas declarado pelo addon, com imagens individuais ou
  uma folha de imagens recortada por `#xywh=x,y,largura,altura`.
- HLS VOD (`.m3u8`) com `EXT-X-IMAGE-STREAM-INF` apontando para uma playlist
  `EXT-X-IMAGES-ONLY`, inclusive folhas descritas por `EXT-X-TILES`.
- JPEG, PNG ou WebP estático, dentro dos limites de tamanho e resolução.

As URLs relativas são resolvidas a partir do documento recebido, incluindo
redirecionamentos. Os cabeçalhos da fonte só acompanham pedidos à origem
do vídeo; não são repassados a outra origem/CDN.

Para addons que disponibilizam WebVTT, o StreamMax reconhece estas extensões
explícitas na resposta de cada stream (também aceitas em `behaviorHints`):

```json
{
  "url": "https://provedor.example/filme.mkv",
  "thumbnailVtt": "https://provedor.example/filme/miniaturas.vtt"
}
```

Também aceita `"thumbnails": {"type": "text/vtt", "url": "miniaturas.vtt"}`.
Esses campos não são obrigatórios no protocolo de addons: **não se presume
que Torrentio, TorBox ou qualquer outro addon forneça essas imagens**.
Um cartaz estático (`thumbnail`/`poster`) não é usado como prévia de cena.

## Fluidez e limites

A descoberta, o download e a decodificação de imagens acontecem em segundo
plano, com apenas um trabalho de recorte por vez. Não há outro player ou
decodificador de vídeo. Cada quadro enviado à interface tem 320 × 180 pixels.
Há uma folha decodificada em cache, liberada ao terminar a busca, e um arquivo
comprimido para evitar repetir downloads durante a mesma reprodução.

O limite é de 2 MiB por documento, 6 MiB por imagem e 6 milhões de pixels por
folha. O decoder pode usar memória temporária adicional. Pedidos têm prazo
de 4,5 segundos, limites de redirecionamento e cancelamento na troca de fonte
ou ao fechar o player. Imagens antigas não são reaproveitadas em outro vídeo.

Sem uma sequência compatível, ou se ela falhar, o avanço e o retorno continuam
normalmente, sem miniaturas. Não se sondam arquivos MP4/MKV nem se baixam
trechos do filme para extrair quadros. Playlists de imagens ao vivo/DVR,
criptografadas ou com byte ranges ficam fora desta implementação.

## Verificação

`bash legacy-source/tests/streammax-source-thumbnails.sh` testa os parsers.
Com SDL2, SDL2_image e cabeçalhos GLES instalados,
`SM_PLAYER_TESTS=1 SM_SANITIZERS=1 bash legacy-source/tests/streammax-source-thumbnails.sh`
exercita o backend real com servidor HTTP local, redirecionamentos entre duas
origens, cancelamento, cache e recortes de uma folha PNG com três cores.
O teste lê os pixels enviados à GPU simulada; não depende de uma janela.

Validação na TCL P7K e com os provedores reais do usuário ainda é necessária.
