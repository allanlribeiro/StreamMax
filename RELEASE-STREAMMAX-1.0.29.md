# StreamMax 1.0.29 — Sincronizar pela fala

Agora e possivel marcar uma fala do filme ou episodio e selecionar a frase
correspondente na legenda para alinhar o tempo, como no Nuvio original.

**Como usar:** selecione uma legenda de texto de um addon. Durante a reproducao,
abra **Legendas > Sincronizacao > Sincronizar pela fala**. Quando ouvir o inicio
da fala, pressione OK. Escolha a frase com cima/baixo e confirme com OK.

- A frase selecionada aparece inteira no painel de leitura, com quebra de linha.
  Esquerda/direita rola esse texto quando ele for maior que a area disponivel.
- **Voltar** permite marcar outra fala; Voltar novamente retorna a sincronizacao.
- **Desfazer ajuste pela fala** restaura a sincronizacao anterior desta faixa.
- O controle **Atraso** continua disponivel para ajuste fino.
- O ajuste vale nesta reproducao e legenda. Trocar a fonte, o titulo ou a faixa
  invalida a selecao anterior; o sincronismo de outros filmes nao e alterado.
- A escolha pela fala tem prioridade sobre o automatico ate ser desfeita.

Primeiro suporte: legendas externas de texto SRT, VTT e ASS. Legendas embutidas,
PGS/VobSub de imagem e canais ao vivo ficam fora desta modalidade. O arquivo de
legenda ja carregado e reutilizado; nao ha reconhecimento de voz nem novo player.

**Validacao:** testes com parser, documento, calculo, ponte de sincronizacao e
painel reais, usando transporte/player/desenho controlados; casos de geracao,
troca de fonte, Unicode, frases longas, sinais do atraso, desfazer, controle e
ponteiro. ASan/UBSan, regressao do app e verificacao do APK. Navegacao completa,
sincronia com fala real e desempenho na TCL ainda dependem do teste no aparelho.

APK: `StreamMax-1.0.29-android.apk`, pacote `com.streammax.tv`, codigo 20036,
assinatura oficial para atualizar a instalacao existente pelo aplicativo.
