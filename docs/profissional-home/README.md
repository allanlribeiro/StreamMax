# Home Profissional — opção adicional

Candidato StreamMax **1.0.27**, código Android **20034**, interno **2.0.34**.
A publicação é confirmada separadamente no registro de validação.

Seleção: **Ajustes → Tela inicial → Layout da home → Profissional**.
Moderna, Padrão, Dinâmica e Globoplay conservam seus índices e sua preferência
local. A atualização não escolhe Profissional automaticamente.

## Prévias

- [Destaque e atalhos](inicio.png)
- [Seleção de coleção](colecoes.png)
- [Fileiras após descer](fileiras.png)

Os PNGs são prévias técnicas 1920×1080, não fotografias da TCL nem capturas
completas do aplicativo. O Hero, os degradês, a composição da superfície de
vídeo, os serviços e os atalhos são renderizados com as funções reais de
`home_profissional.inc` e `gfx.c` inteiro. Menu, rasterização dos textos,
catálogo, logos de exemplo, histórico e fileiras são entradas de demonstração.
As marcas ilustradas não confirmam assinaturas ou fontes disponíveis na conta.

## Comportamento

- Hero integrado ao fundo, sem painel arredondado ou vizinhos tocando vídeo.
  Cover uniforme e um único decoder. Degradê horizontal para leitura, topo
  discreto e base concentrada nos últimos 22% da imagem.
- Até quatro coleções visíveis/configuradas, priorizando o grupo Streaming.
  Logo da própria coleção, com título quando não houver logo. Sem coleções,
  aparecem atalhos dos catálogos de filmes/séries que já existem na Home.
- Atalho TV ao vivo abre o Guia TV existente; VOD depende do Xtream
  configurado e dos recursos oferecidos pelo servidor. Nenhum motor IPTV novo.
- Serviços do catálogo aparecem apenas quando os metadados do próprio título
  os informam. Não se infere disponibilidade a partir do trailer ou da marca
  de uma coleção. Escolha de fontes dos addons, aluguel e compra continuam
  na página do conteúdo.
- D-pad: destaque → atalhos → fileiras; para cima faz o caminho de volta.
  Esquerda no primeiro atalho abre o menu. OK em coleção/catálogo abre seu
  destino, sem publicar um índice de filme; segurar OK não abre o filme do Hero.
- O ponteiro tem alvos por atalho, sem camada cobrindo o Hero. As coleções
  são reconstruídas apenas quando as revisões existentes mudam.
- Cache e player existentes; sem GIF de coleção, desfoque extra ou consulta
  de streams por quadro. Uma composição de mídia por quadro em repouso.

## Validação

`legacy-source/tests/streammax-profissional-home.py` compila o renderizador
GLES completo e as funções reais do novo layout. Verifica cover para fontes
16:9/cinema/quadradas/retrato/desconhecidas, composição sem foto residual,
gradientes monotônicos e escalas 85–150%. As escalas são verificadas no shader;
não equivalem a uma captura de cada tela completa em cada escala.

As rotas, limites, ausência/duplicatas de coleções, fallback para catálogos,
controle remoto e callback do ponteiro são exercitados com ASan/UBSan e dados
controlados. `ajustes.c` inteiro verifica valores locais, índices antigos,
cinco opções, menu superior e fallback inválido. Não é teste de login real ou
de sincronização de conta pela rede.

Também são exigidas as regressões do Globoplay, Moderna, abertura fria/quente
do trailer, rolagem e ausência das miniaturas de avanço canceladas.
Compilação Android, assinatura e publicação ficam registradas após conclusão.

**Limites:** sem TV TCL conectada, teste físico de navegação/reprodução,
medição de FPS, servidor Xtream do usuário ou credenciais reais de login.
