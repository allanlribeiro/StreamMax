# Abertura do trailer antes da primeira imagem

O usuário informou que a 1.0.23 ainda apresenta faixas no topo/direita. A nova foto mostra o vídeo terminando aproximadamente em x=1216 numa imagem de 1280 px. Para a caixa `(537,0,1383,670)` da 1.0.23, FIT com vídeo 16:9 termina aproximadamente em x=1824 no canvas 1920, ou x=1216 na captura. Essa correspondência motivou verificar a ordem real da abertura.

## Falhas encontradas

1. `home_trailer_passo` chamava `trailer_abrir` e só depois `trailer_marcar_dono(HOME)`. Durante a primeira abertura, `nativoJanela` ainda via `dono=NENHUM` e enviava FIT. Os testes anteriores chamavam o helper já com `dono=HOME`; por isso não cobriam esse primeiro comando.
2. `NvPlayer.definirJanela` descartava o pedido quando `player==null`, situação possível durante a espera assíncrona pela liberação do decoder anterior. O dedupe C podia considerar o pedido entregue.
3. Mesmo um pedido posterior correto não garantia que a primeira superfície fosse construída nesse enquadramento: a URL e a janela eram mensagens separadas, e `abrirMain` aplicava o encaixe inicial antes de a segunda mensagem ser processada.

O código existente já documenta que certas TCL podem conservar a geometria inicial da superfície. Não houve acesso aos logs/compositor desta TV; o teste comprova o primeiro comando errado, sem declarar medição física dessa retenção.

## Correção

- `trailer_abrir_home` define o dono e o destino antes de abrir e não toma um trailer ativo de detalhes. Falha limpa o contexto.
- `nativoTocar` usa `video_android_tocar_home` apenas para o destaque Moderno em Android TV. Cartão, detalhes e outros destinos mantêm seu caminho anterior.
- `abrirComJanela` recebe URL, geração e geometria na mesma chamada JNI. A mensagem do Handler guarda a janela antes de entrar em `abrirMain`.
- `Pending` mantém a janela durante o release. Ela só é consumida pelo mesmo pedido e é aplicada depois de criar o player e antes de anexar sua Surface. `aplicarEncaixe` só ocorre quando não existe janela daquele pedido.
- O caminho inicial do Home não faz fallback para uma assinatura JNI antiga sem janela; falha de ponte mantém a arte em vez de iniciar com geometria inadequada.
- Não foram alterados degradê, escala, posição dos cartazes, fontes de trailer ou política de buffer. Não foi introduzido outro decoder ou caminho TextureView.

## Verificações

- Baseline `554e8577e70f5211520e7ad4572f70838e1b8180`: compila o `trailer.c` completo e reproduz FIT como primeiro comando antes da marcação de dono.
- Candidato: arquivo completo, backend controlado, relógio simulado, ASan/UBSan; abertura fria/quente, metadata ausente/tardia/cinema, atualização da caixa, encerramento e falha. Todos os comandos do destaque são COVER; cartão/detalhes mantêm seus modos.
- Classe Java usada pelo player: janela retida, consumo único, último pedido prevalece e geração antiga descartada. Contratos conferem a aplicação antes de `setVideoSurfaceView`.
- Verificação do APK real: método público estático `NvPlayer.abrirComJanela` e descritor `(Ljava/lang/String;Ljava/lang/String;IIIIIII)V` no DEX e nas duas bibliotecas nativas.
- Regressões do renderizador completo e da rolagem continuam sendo executadas, sem mudar seus parâmetros.

Limites: backend/relógio controlados não são MediaCodec ou compositor de uma TCL. Não foi feito teste físico nem medição de FPS. Tarjas embutidas no próprio arquivo permanecem uma condição separada da janela Android.
