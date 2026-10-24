# StreamMax Legacy 1.0.16 — ampliar destaque

Pedido após teste na TCL: aumentar o trailer 17% para a esquerda, 3% para baixo e aumentar o degradê 14% sobre a versão 1.0.15.

## Resultado

- Ponto de partida: viewport da 1.0.15 calculado pela proporção da fonte, limitado ao hero. Largura multiplicada por 1,17 e altura por 1,03, canto superior direito fixo.
- Fonte 16:9 em layout virtual 1920×1080: antes 778,67×438 em x=1141,33; agora 911,04×451,14 em x=1008,96. A base desce 13,14 pixels; a esquerda avança 132,37 pixels.
- Imagem escalada uniformemente para preencher o viewport, sem esticar. Como a ampliação horizontal é maior que a vertical, há enquadramento vertical de aproximadamente 12% no total, distribuído entre topo e base. Máscara e destino do viewport são compartilhados.
- Largura relativa do degradê esquerdo passa de 16% a 18,24%; faixa inferior passa de 14% a 15,96% (14% de aumento relativo, não 14 pontos percentuais). Centro, topo e direita sem véu adicional.
- Legendas dos trailers portugueses reposicionadas acima do degradê inferior; usam a SubtitleView existente. Filmes e episódios mantêm suas próprias preferências.
- Ícone, abertura, addons e menu da 1.0.15 preservados.

## Idioma

A Apple fornece variantes de áudio/legenda, cuja seleção pt-BR é preservada. A fonte IMDb atual entrega MP4 direto. Consultas públicas a 24–32 candidatos em Vingadores: Ultimato, Avatar: O Caminho da Água, Homem-Aranha: Através do Aranhaverso e Barbie não identificaram versões dubladas/legendadas portuguesas nos metadados. Não foi declarado suporte geral a dublagem do IMDb nem acrescentada extração do YouTube. A disponibilidade em português fora da Apple continua uma limitação; a atualização visual não cria faixas ausentes.

## Validação e distribuição

Testes C de crescimento, ancoragem, cobertura e proporção real da imagem; degradê com aumento relativo exato; regressões HLS/idioma, Home/addons/abertura/updater e gates de playback/autenticação/isolamento. Compilação Android ARM64/ARMv7 e certificado oficial pelo workflow antes da publicação. Mesmo pacote com.streammax.tv, versão 1.0.16, Android code 20023. Publicar pelo atualizador existente após conferir APK e integridade, conforme autorização persistente do proprietário. Enquadramento e fluidez ainda requerem teste físico na TCL.
