# Sincronizar pela fala — StreamMax 1.0.29

Selecione uma legenda de texto de um addon, abra **Legendas > Sincronizacao >
Sincronizar pela fala** e pressione OK quando ouvir o inicio de uma fala.
Escolha a frase correspondente com cima/baixo; confirme com OK para alinhar.

A frase focada aparece inteira no painel inferior, com quebra de linha. Use
esquerda/direita para ler as partes que excederem a altura disponivel. O painel
usa a ilha existente do player, ampliada e ancorada a esquerda. Ponteiro tambem
permite marcar e selecionar; hover apenas navega, clique aplica.

Voltar da lista retorna a marcacao; Voltar novamente cancela. Depois de aplicar,
**Desfazer ajuste pela fala** retoma a sincronizacao anterior/automatica. **Atraso**
continua como ajuste fino. O instante capturado permanece a referencia enquanto
o filme continua; escolher uma frase nao avanca nem reabre o video.

## Escopo e implementacao

- Apenas a legenda principal externa de texto: SRT, VTT e ASS. Embutidas,
  legendas de imagem (PGS/VobSub) e canais ao vivo ficam fora desta modalidade.
- `legfala.c`: retencao de documento imutavel, indice de falas nao vazias,
  busca binaria da frase proxima, preservacao de UTF-8 e calculo do atraso.
  Prepara a leitura completa com a medida real da fonte, inclusive palavras
  sem espacos: cada linha respeita a largura e o limite do componente nativo.
  A quebra fica em cache por frase, largura e abertura do painel.
- `legendasui_fala.inc`: duas etapas, leitura completa com rolagem, controle,
  ponteiro, bloqueio de OK segurado/clique antigo e mensagens em portugues.
- `legsync.c`: guarda de documento, geracao e URL da midia, prioridade do ajuste
  da pessoa sobre escala/offset automaticos e desfazer. Troca de faixa/fonte ou
  fim da sessao limpa a correcao; nao altera preferencia global nem outros filmes.
- Reutiliza o documento ja carregado; sem novo download de legendas ao abrir,
  reconhecimento de voz, segundo player ou mudancas na Home. Desenha somente
  as linhas visiveis e a frase focada. Nao foi medido FPS no aparelho.
- Formula do Nuvio: instante capturado menos inicio da frase menos 300 ms de
  compensacao de reacao; ajuste fino manual preservado. O sinal e convertido
  para o compositor nativo uma unica vez. Limite +/-60 s: recusa em vez de
  aplicar um valor cortado silenciosamente.

Referencia do Nuvio original: [d14832e](https://github.com/NuvioMedia/NuvioTV/tree/d14832e1c477cca2b5613b893e590fd5fcab83ee/app/src/main/java/com/nuvio/tv/ui/screens/player).
A interface Compose nao foi usada como segundo player; o comportamento foi
adaptado ao nucleo C/SDL/GLES e ao menu de legendas da base de producao.

## Validacao e limites

`streammax-speech-sync.sh` executa parser/documentos reais SRT/VTT/ASS, modelo,
ponte de sincronizacao e o painel de producao com coletores/player/desenho
controlados. Cobre atrasos positivos/negativos/zero e limite, frase longa,
Unicode, arquivo incompleto, reapertura do mesmo documento, captura historica,
OK segurado, ponteiro antigo, cancelamento, faixa/fonte/sessao diferentes,
resultado automatico tardio, ajuste fino, desfazer e geometria 85-150%.
Tambem cobre palavras de 767 bytes, acentos, emojis/CJK, preservacao dos
caracteres, largura maxima de linha e buffers pequenos, sem corte de UTF-8.

ASan/UBSan e sintaxe nativa completos; regressao de cadastro, contas, addons,
VOD/guia, Watch Next, retirada de frames, Home, logo, pacote e OTA. O workflow
tambem executa a integracao de menus `streammax-track-panels.sh` e confere a
presenca da funcao nos dois binarios nativos do APK.

**Pendente no aparelho:** navegação completa, leitura com as fontes e idioma
reais, sincronizacao com audio real, formatos de servidores pessoais e fluidez
na TCL. Os testes de desenho controlado nao sao capturas do APK/emulador.
