# StreamMax — interface IPTV (decisão de 21/10/2026)

## Decisão aprovada
Na seção atual de TV ao vivo, utilizar **três abas horizontais no topo**:
1. **Canais ao vivo**
2. **Filmes IPTV**
3. **Séries IPTV**

Decisão provisória, sujeita a ajustes após teste na TCL P7K. Não criar uma nova seção independente na Home. Não misturar o catálogo IPTV com os catálogos dos addons.

## Navegação Android TV
- Controle remoto: esquerda/direita entre abas quando o foco está no cabeçalho; OK ativa a aba; baixo entra no conteúdo da aba.
- Restaurar foco por aba ao retornar; voltar primeiro sai do detalhe/player para a aba anterior e depois respeita a navegação já existente.
- Mostrar estado de carregamento discreto, vazio e erro com opção de tentar novamente; não bloquear as outras abas quando uma falhar.
- Manter indicador de foco bem visível e identidade azul cinematográfico; sem GIFs ou animações pesadas.

## Conteúdo
- **Canais ao vivo:** preservar navegação existente e velocidade de troca; categorias e canais sem redesenho desnecessário.
- **Filmes IPTV:** categorias do provedor e grade de pôsteres; detalhe com metadados disponíveis e reprodução.
- **Séries IPTV:** categorias e pôsteres; detalhe com temporadas e episódios quando fornecidos pelo provedor.
- Exibir somente conteúdos que a lista IPTV configurada disponibiliza. Não inventar dados ausentes.

## Desempenho e isolamento
- Carregar categorias/itens somente quando a aba for aberta; paginação ou lotes sob demanda e imagens com cache limitado.
- Não carregar catálogos inteiros em memória, nem iniciar consultas VOD ao abrir Canais ao vivo.
- Se não houver VOD no provedor, exibir estado vazio sem prejudicar canais.
- Preservar credenciais, URL e reprodução existentes; nenhuma alteração de assinatura nesta etapa.

## Critérios de aceitação
- Trocar de aba com D-pad e voltar sem perder foco.
- Filmes e séries IPTV não aparecem nas linhas dos addons.
- Catálogos grandes não congelam a interface nem provocam carregamento massivo de imagens.
- Abrir canais ao vivo não aguarda carregamento do VOD.
- Validar visualmente e medir memória/fluidez na TCL P7K antes de promover para release.

Este documento registra a escolha de UX; não afirma que a implementação ou os testes foram concluídos.
