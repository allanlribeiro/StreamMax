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

## Revisão do código e CI — 21/10/2026
- O caminho de foco está implementado em `legacy-source/src/guia.c`: ESQUERDA da coluna de categorias entra no cabeçalho; ESQUERDA/DIREITA alternam a aba focada; OK ativa; BAIXO retorna ao conteúdo. A tecla azul é atalho opcional, não requisito para controles TCL.
- A interface de abas e o desenho do VOD estão em `legacy-source/src/streammax_iptv_vod.c`; não reconstruir esses componentes.
- Execuções GitHub Actions `39579537506` e `39579530894` (IPTV/idiomas) e `39579537331` (regressão) concluídas com sucesso. Isso **não** substitui teste real de foco, reprodução e memória na TCL.
- O `PR Template Check` da PR #32 ainda falha (`39579537538`): a descrição referencia aprovação em conversa, mas a política para mudança maior exige issue vinculada; issues estão desabilitadas conforme a própria PR. Não mascarar a falha nem mudar a categoria da alteração só para passar no CI. Resolver a exigência de governança antes do merge.
- Abertura de filmes, episódios e retorno ao guia, consumo de memória e fluidez com catálogos grandes permanecem **não verificados no dispositivo**.

## Limite identificado em catálogos grandes (revisão de código)
A implementação atual de `streammax_iptv_vod.c` aloca no máximo **48 categorias, 96 itens e 96 episódios** por consulta (`VOD_CAT_CAP`, `VOD_ITEM_CAP`, `VOD_EP_CAP`). A primeira entrada busca apenas categorias; a escolha da categoria inicia consulta em worker. Isso limita memória, **mas não equivale a paginação**: se o provedor tiver mais títulos do que o limite, os excedentes podem não aparecer. Não anunciar suporte completo a listas enormes antes de verificar a API e implementar navegação por páginas/lotes, se viável.

A troca de aba invalida a geração dos resultados antigos, evitando exibir resposta desatualizada. Contudo, o worker ativo ainda pode ocupar a única vaga de consulta até terminar; medir a latência de troca de abas com rede lenta. Nenhum teste em dispositivo ou correção de paginação foi realizado nesta revisão.

## Verificação do contrato Xtream e plano de paginação (21/10/2026)
A consulta existente usa `get_vod_categories`/`get_series_categories` e, ao abrir uma categoria, `get_vod_streams&category_id=...` ou `get_series&category_id=...`. **O código não envia `page`, `offset` ou `limit` para o painel**. O limite de 96 é apenas do parser local: a resposta JSON completa já foi recebida em memória, e os títulos posteriores são ignorados. Portanto, a solução não é simplesmente elevar `VOD_ITEM_CAP` nem afirmar que a rede está paginada.

Próxima implementação recomendada: criar uma coleção local limitada por lotes, com paginação/rolagem sob demanda e descarte dos lotes antigos, após verificar se o provedor aceita paginação no servidor. Se não aceitar, avaliar cache temporário compacto por categoria e parser incremental/streaming para evitar manter JSON gigante na RAM; a interface deve sinalizar quando o catálogo está truncado até a solução completa. Para categorias acima de 48 e temporadas acima de 96 episódios, aplicar o mesmo princípio. Testar contra respostas grandes e lentas sem registrar URLs nem credenciais.
