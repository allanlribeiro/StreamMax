# Bordas do trailer na TCL P7K — 1.0.23

O usuário confirmou a foto da versão 1.0.22: corte abrupto inferior/lateral esquerda e faixa preta à direita. A validação da 1.0.22 comparava colorStops em um shader isolado, sem validar sua posição relativa aos cartazes nem o modo inicial da janela Android.

## Mudança aplicada

- Caixa com 72% da largura, ancorada na borda direita real da TV. A arte nativa podia ultrapassar essa borda em 56 px; ela não serve mais como limite de vídeo.
- Conversão explícita da escala de texto entre as coordenadas virtuais da Home e o canvas do player. Antes, o desenho e a janela podiam receber medidas diferentes quando a escala não era 100%.
- A área de vídeo permanece grande; o degradê termina no topo dos cartazes (`378 + 48 = 426` no canvas padrão), não no final da arte de 670 px. A faixa inferior ocupa até 30% da altura visível, limitada a 144 px. Quando o foco no destaque desce os cartazes, a transição acompanha essa borda.
- Curva quintica de transparência na esquerda e na base, com precisão alta onde suportada. O miolo e a borda direita permanecem livres da máscara.
- Quando existe foto no fundo inteiro, a área à esquerda do vídeo também dissolve para a mesma cor da primeira coluna da rampa, evitando emenda vertical entre a foto e o degradê. É um retângulo de cor simples, sem nova textura ou blur.
- Modo Android COVER=2, preservado como inteiro no JNI/Kotlin. Usado desde a abertura do trailer da Home; metadados tardios recalculam o preenchimento. Reaplicar o recorte preserva COVER. Os modos antigos EXACT=0 e FIT=1 continuam disponíveis para os demais destinos.
- Cover/overscan uniforme da 1.0.22 mantido. Sem textura extra, blur, framebuffer adicional ou segundo decoder.

## Evidência e limites

O teste usa `gfx.c` completo, compilando os programas reais, o vertex shader, os uniforms e o blend. Extrai as funções reais de geometria da Home e de despacho/reaplicação da janela nativa. Compila também a classe Java usada em `NvPlayer.calcularJanela`.

Com o `gfx.c` publicado em `af60fad69e95c3c3d95ca8ca325afb5f4e7ad020`, o teste reproduz transparência total na linha dos cartazes, pois o degradê só terminava abaixo deles. Para a caixa antiga `(555,0,1421,670)` e vídeo 1920×1080, FIT deixa aproximadamente 59 px lógicos sem vídeo na direita, equivalentes a 39 px em 1280×720, compatível com a faixa da foto. Isso comprova o comportamento do código, sem atribuir todos os detalhes do hardware a uma única causa.

O candidato verifica pixels do renderizador completo em escalas de 85%, 95%, 100%, 105%, 110%, 125% e 150%, foco na fileira e no destaque: borda direita livre da máscara, fechamento opaco antes dos cartazes, transições monotônicas esquerda/inferior, limites de salto entre pixels e crossfade. Verifica abertura COVER com dimensões desconhecidas/16:9/cinema/quadradas/verticais, reaplicação e preservação de cartões/detalhes/tela cheia/telefone. O primeiro quadro continua obrigatório para revelar o trailer.

Não houve acesso remoto à TCL nem medição física de FPS. A captura fornecida é evidência do problema; os testes usam uma cena controlada e não são uma fotografia da TV corrigida. Tarjas gravadas dentro do próprio arquivo não são detectadas dinamicamente nesta etapa.
