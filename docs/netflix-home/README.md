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


## Entrega verificada

- [APK de teste assinado](https://github.com/allanlribeiro/StreamMax/releases/download/v1.0.31-netflix-preview/StreamMax-1.0.31-android.apk)
- [Release e hashes](https://github.com/allanlribeiro/StreamMax/releases/tag/v1.0.31-netflix-preview)
- [Manifesto da validação](./VALIDACAO-1.0.31.json)
- Compilação e testes: workflow `43589074808`, conclusão **success**.
- APK: 64.065.140 bytes; SHA-256
  `f2d79423c32c7ca67f5abff2961e9e6e1f04f86ad1a22341a5f9ab72ab45f1b4`.
- Os quatro anexos da release foram comparados por tamanho e SHA-256.
  A publicação foi concluída por edição de metadados após a consulta do draft
  pelo endpoint de tag retornar 404; o APK não foi recompilado ou alterado.
- Release marcada como prévia; `/releases/latest` permaneceu em `v1.0.30`.


## Atualização automática — disponibilizada a pedido do usuário

A versão **1.0.31** está publicada como release oficial e retornada por
`/releases/latest`. O APK é idêntico ao da prévia assinada, sem recompilação.
O atualizador C original da 1.0.30 foi executado contra o JSON público real e
reconheceu a versão, o arquivo correspondente e o SHA-256 corretos. O link
público do APK também foi conferido (HTTP 206, cabeçalho APK válido).

[Release oficial 1.0.31](https://github.com/allanlribeiro/StreamMax/releases/tag/v1.0.31)
· [Registro da validação OTA](./VALIDACAO-OTA-1.0.31.json).

A confirmação da instalação continua sendo solicitada pelo Android na TV.
A verificação em uma TCL física continua pendente. Os registros de prévia
acima descrevem o estado anterior à promoção para o atualizador automático.
