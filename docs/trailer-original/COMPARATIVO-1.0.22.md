# Apresentação do trailer: Nuvio original e StreamMax

Referência consultada em 27/10/2026: [NuvioMedia/NuvioTV, commit 252e9eb0ceb3d31d7bcbe770736f6eb83f283e05](https://github.com/NuvioMedia/NuvioTV/tree/252e9eb0ceb3d31d7bcbe770736f6eb83f283e05).
Arquivos `ModernHomeHero.kt`, `ModernHomeModels.kt`, `ModernHomeContent.kt` e `TrailerPlayer.kt`, em `app/src/main/java/com/nuvio/tv/ui/`.
O original é Kotlin/Compose; o StreamMax publicado usa o núcleo Legacy C/SDL/GLES. O ajuste adapta a apresentação à geometria nativa existente.

| Aspecto | StreamMax 1.0.21 | Nuvio original | Ajuste 1.0.22 |
|---|---|---|---|
| Área do vídeo | Calculada com largura/altura da fonte e limites 64% da tela/438 px, depois aumentada 17%/3% | Mesma caixa da imagem de fundo, independente da fonte | A caixa da arte nativa também é a caixa do trailer |
| Enquadramento | Cover uniforme dentro da área reduzida | Crop/cover e overscan uniforme 1,35 | Cover e overscan 1,35, preservando proporção |
| Degradê na faixa | Smoothstep de 18,24% à esquerda e últimos 15,96% na base | 45% à esquerda; base a partir de 82%; paradas lineares | Mesmas coberturas e paradas do original |
| Fundo inteiro | Mesma máscara estreita do canto | 65% à esquerda; base a partir de 64%; o modo cinema remove as rampas enquanto toca | Rampas correspondentes à caixa inteira; preservada a Home visível, sem ativar cinema |
| Cor das bordas | Foto parada dissolvida sobre o vídeo | Cor do fundo da interface | Cor do fundo nativo, sem foto residual no término |
| Entrada | Mola exponencial; condição pronta também podia aceitar timeout sem imagem | Transição de 480 ms após primeiro quadro | 480 ms, limitada a [0,1], após quadro real; sem animação quando reduzida |

A caixa nativa na faixa é a já usada pela arte: `(555,0,1421,670)` no canvas 1920×1080. O original Android calcula sua caixa em dp (72% da largura com deslocamento pelo tema; altura depende das fileiras). Não se afirma igualdade pixel a pixel entre runtimes. O fundo inteiro usa a caixa já escolhida pela Home. Preferências e fileiras não são migradas nem reordenadas.

## Composição

O quad do destaque produz cor/alfa pré-multiplicados. Para progresso `a` e opacidade combinada do degradê `g`, o canvas fica `arte*(1-a) + fundo*(a*g)`, com alfa `(1-a)+a*g`. O compositor completa com `video*a*(1-g)`. Ao terminar, a foto não participa mais. Uma passagem, sem textura extra, blur, FBO ou segundo decoder.

## Verificação

- `streammax-hero-original.py`: extrai as funções reais da Home e do trailer e o shader/compositor reais de `gfx.c`.
- Geometria normal e inteira, fontes 16:9, 2,39:1, quadradas, verticais e dimensões ainda desconhecidas; cover uniforme e ancoragem central.
- Primeiro quadro, detalhes/cartões preservados, Padrão e Dinâmica preservados.
- Mesa GLES sem janela: 273.920 valores de canais comparados a interpolação genérica dos `colorStops` do Compose; cinco progressos de entrada, limites do retângulo, faixa e fundo inteiro; restauração dos fatores de blend.
- Testes de áudio/legendas HLS, transição a 30/60/120 Hz simulados, pausa longa e animações reduzidas.
- Regressões de rolagem real com ASan/UBSan, retirada dos frames, addons, VOD e atualizador.

Limites: não houve teste físico da TCL P7K nem medição de FPS. O original também possui detecção dinâmica de tarjas via TextureView; ela não foi portada para a SurfaceView nativa. Tarjas gravadas no próprio vídeo podem variar entre fontes.
