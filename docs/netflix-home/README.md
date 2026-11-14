# Modo Netflix — StreamMax 1.0.31 (teste)

Base: versão oficial 1.0.30, commit `eb9f182b9875cf21cf6ead301066f40294475b70`.

## Ativar e voltar

Na TV, abra **Ajustes → Tela inicial → Layout da home → Netflix**. A opção é adicional;
Moderna, Padrão, Dinâmica, Globoplay e Profissional continuam disponíveis. Não é
necessário recadastrar conta, perfis, fontes ou biblioteca.

## Apresentação

- Navegação horizontal no topo, utilizando os destinos reais já existentes.
- Destaque amplo emoldurado, sinopse e ações **Assistir/Retomar** e **Mais informações**.
- Cartazes verticais grandes: base de 348 × 522 em uma interface de 1920 × 1080.
  A escala da interface preserva o tamanho físico aproximado dos cartazes.
- O cartão focado em Continuar assistindo se expande para aproximadamente
  928 × 522, com progresso vermelho, episódio e minutos restantes quando
  esses dados existem. Os vizinhos continuam verticais.
- Foco branco, animação compartilhada das fileiras e memória de coluna.
- Esquerda/direita alternam as duas ações do destaque; direita após Mais
  informações passa ao próximo destaque. Cima acessa o menu; baixo, as fileiras.
- OK curto no destaque aciona a ação selecionada; OK longo mantém o menu do título.

O modo utiliza o catálogo, cache, artes, logos e rotas de reprodução existentes.
Não consulta a conta Netflix nem inventa assinaturas, classificações ou
informações indisponíveis. Sem logo, o título usa texto. Sem arte, conserva o
estado de carregamento da Home. A reprodução e a resolução de próximo episódio
continuam no mesmo fluxo do aplicativo.

O trailer do destaque usa o único decoder já existente e fica dentro da moldura.
Não foi adicionado outro player, vídeo de fundo permanente ou novo efeito blur.
Os trailers automáticos dos cartões de Continuar assistindo continuam seguindo
as restrições já existentes do aplicativo.

## Verificação

Executados em Linux, com código C/GLES de produção:

- Renderização do destaque e Continuar assistindo, sem erros GL.
- Geometria em quatro escalas (85–150%), 1.500 combinações de rolagem/expansão,
  ações reais do destaque e memória de coluna.
- Seis opções de layout, índices/default antigos e valores inválidos.
- Sintaxe completa de home, menu, continuar, ajustes, gfx e trailer.
- Regressões de renderização Globoplay/Profissional, janela Android de trailer,
  rolagem, Watch Next, atualização, fontes, cadastro, player e sincronização.

As capturas desta pasta são **prévias nativas de componentes**, com catálogo,
textos e imagens de exemplo. Não são capturas de uma TCL conectada ou de uma
conta real. A prateleira nas capturas é uma fixture; a implementação usa as
fileiras reais da Home.

Ainda exige validação na TCL física: fluidez/FPS e memória durante navegação
prolongada, reprodução real, retorno do player, catálogo remoto, áudio do trailer,
controle remoto, troca de perfil e sincronização entre TVs. Nenhum desses testes
físicos é declarado como concluído.

## Compilação e distribuição

Workflow `streammax-netflix-preview.yml`: testes, compilação dos dois ABIs ARM,
verificação de pacote `com.streammax.tv`, versionCode 20038 e comparação do
certificado com o APK oficial 1.0.30. Utiliza os segredos de assinatura já
existentes no repositório, sem expor a chave. O workflow apenas gera artefatos;
não altera a release estável nem o feed OTA.
