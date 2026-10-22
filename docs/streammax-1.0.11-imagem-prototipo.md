# StreamMax 1.0.11 — melhoria leve da ampliação HD

Candidata Android assinada, baseada no trabalho 1.0.10 e no piloto 1.0.11. Não foi homologada em TV real. Não publica OTA automaticamente.

## Uso
Ajustes → Reprodução → Imagem e som → **Upscaling: melhoria de imagem HD**.
Opções: **Desativado** (padrão) e **Automático (experimental)**. Vale no próximo vídeo.

A melhoria é nitidez leve em resolução de origem para ajudar o scaler nativo da tela. Não é IA, super-resolução ou recuperação de detalhe perdido pela compressão. O shader tem uma passagem, cinco amostras e uma textura de saída na resolução de origem; não aloca intermediário 4K.

## Admissão antes de criar GL
Media3 1.8.0 recebe todos os formatos selecionáveis em `Renderer.enable`. `StreamMaxImageRenderer` verifica esses formatos antes de delegar ao renderer real e só então configura efeitos. A produção nunca chama `ExoPlayer.setVideoEffects`, nem com uma lista vazia: essa chamada tornaria a lista de efeitos não nula e poderia criar um VideoSink GL para conteúdo protegido.

Só permite SDR explicitamente identificado, 1280–1920 de largura e 480–1080 de altura (inclui HD com barras cortadas), FPS conhecido até 30, pixels quadrados, sem rotação/DRM e tela maior nos dois eixos. Exclui Dolby Vision por MIME e codecs dvhe/dvh1/dvav/dva1, PQ, HLG, BT.2020, UHD e metadados desconhecidos. Todos os formatos adaptativos selecionáveis precisam passar. Tunneling nunca ativa o efeito. A seleção de decoder, áudio e fonte segue a política existente.

`onStreamChanged`, `onInputFormatChanged`, `maybeInitializeProcessingPipeline` e `onOutputFormatChanged` verificam mudanças antes de enviar o novo formato ao processamento. Uma mudança incompatível interrompe o renderer antes do processamento e aciona nova sessão nativa.

## Desempenho e recuperação
O monitor por sessão recua após duas janelas consecutivas com pelo menos quatro quadros perdidos por segundo. Pausa, contador reiniciado e amostras atrasadas reiniciam a sequência. Pressão térmica moderada ou modo de economia impedem a ativação e provocam recuo se já ativo.

O recuo libera o player e sua superfície GL e reabre a mesma fonte na posição atual, preservando parâmetros de faixas, velocidade, ganho e pausa. Não apenas remove o filtro. O efeito permanece bloqueado nessa fonte até uma nova abertura solicitada pelo usuário, evitando ciclos. Pode haver uma breve interrupção durante o recuo. Próximos episódios preparados são descartados ao mudar a opção ou recuar, para não adotar uma sessão antiga com GL.

## Validação
- Testes Kotlin de admissão SDR, HDR/DV/UHD, formatos adaptativos mistos, dados desconhecidos, display, tunneling, queda sustentada, pausa e pressão térmica.
- Contratos de integração e regressões existentes de atualização, Home, seleção de fonte e legendas regionais.
- Workflow da candidata compila APK nativo completo, roda testes Kotlin, verifica pacote, versão, bibliotecas e assinatura compatível com 1.0.9.
- Pendente na TCL P7K: comparar a mesma cena com OFF/Automático, medir FPS, PSS, temperatura, seek e tempos de abertura; testar transições SDR→HDR/DV/4K e vice-versa, áudio, legendas e retomada. Compilação não comprova melhoria visual nem impacto zero.

Versão comercial: 1.0.11; Android versionCode: 20018. O número interno Legacy 2.0.18 só mantém o versionCode crescente.
