# StreamMax — guia de continuidade do projeto

## Continuidade — Hero 15% mais alto e previa (28/10/2026)

- O usuario aprovou o degrade da 1.0.24 e pediu testes de mais altura para
  revelar a parte inferior do trailer, com envio de imagens.
- Branch `feat/streammax-hero-height`: Home Moderna Android TV com Hero ligado
  ganha 15% de altura ate os cartazes (426 -> 489,9); player enquadrado na area
  visivel (513,9 incluindo reserva de foco), mantendo mascara maior.
- Zoom fixo de 35% substituido por margem minima de 2%, com cobertura
  adicional para fontes ~16:9 com barras de cinema embutidas ate 2.40:1.
  Heuristica de proporcao, nao deteccao de pixels. Demais destinos preservados.
- Renderizador completo em escalas 85-150%, abertura com ASan/UBSan,
  geometria/fontes, rolagem horizontal com sanitizadores e demais verificacoes
  locais passaram. FPS e dispositivo fisico nao foram medidos.
- Previa tecnica com funcoes/shader reais e conteudo estatico demonstrativo,
  nao captura completa do app nem foto da TV: [imagens e limites](./docs/hero-altura-15/README.md).
- Sem APK/OTA novo nesta etapa; versao distribuida continua 1.0.24.

## Continuidade — abertura do trailer após novo teste da TCL (27/10/2026)

- O usuário informou que a 1.0.23 ainda não resolveu o enquadramento.
- Branch `fix/streammax-trailer-open-context`; versão 1.0.24, código 20031.
- Falha reproduzida com `trailer.c` completo da 1.0.23: a abertura ocorria
  antes da marcação de dono HOME, portanto o primeiro comando era FIT.
- Novo `trailer_abrir_home` define contexto antes de abrir; URL e janela
  COVER chegam juntas a `NvPlayer.abrirComJanela`, antes da Surface.
- Janela retida durante release assíncrono, consumida somente pelo mesmo
  pedido. Ponte JNI conferida no DEX e nos dois ABIs do APK.
- Testes de abertura fria/quente, metadata tardia, falha e destinos alternativos
  aprovados com ASan/UBSan. Degradê e layout da 1.0.23 preservados.
- [Análise e limites](./docs/trailer-abertura/ANALISE-1.0.24.md).
- APK compilado e assinado com a chave oficial; publicado no canal do app.
  Atualizador da 1.0.23 conferido com JSON público, URL e SHA-256 do APK.
- [Versão 1.0.24](https://github.com/allanlribeiro/StreamMax/releases/tag/v1.0.24) e
  [registro de validação](./docs/trailer-abertura/VALIDACAO-1.0.24.json).
- Confirmação na TCL e medição de FPS ainda pendentes.

## Continuidade — bordas do trailer após foto da TCL (27/10/2026)

- O usuário confirmou problemas visuais na 1.0.22: direita com faixa preta,
  esquerda sem transição perceptível e base com corte seco.
- Branch `fix/streammax-trailer-edges`; versão 1.0.23, código 20030.
- Quadro ancorado na direita, escala de texto convertida para o player,
  degradê inferior alinhado ao topo dos cartazes e curva suave nas bordas.
- Android passa COVER=2 desde a abertura e preserva esse modo na reaplicação;
  Java/Kotlin usam geometria testada, mantendo EXACT/FIT nos outros destinos.
- Novo teste inclui `gfx.c` inteiro, geometria real e classe Java do player.
  Baseline da 1.0.22 reproduz base tardia; FIT reproduz faixa direita.
- [Análise e limites](./docs/trailer-bordas/ANALISE-1.0.23.md).
- APK compilado, assinado com a chave oficial e publicado no canal do app.
  Atualizador da 1.0.22 validado com o JSON público, URL e hash do APK.
- [Versão 1.0.23](https://github.com/allanlribeiro/StreamMax/releases/tag/v1.0.23) e
  [registro de validação](./docs/trailer-bordas/VALIDACAO-1.0.23.json).
- Resultado físico na TCL e FPS ainda pendentes.

## Continuidade — trailer com referência no Nuvio original (27/10/2026)

- Pedido: comparar o destaque com Nuvio original e ajustar tamanho/degradê.
- Referência: `NuvioMedia/NuvioTV`, `252e9eb0ceb3d31d7bcbe770736f6eb83f283e05`.
- Branch `fix/streammax-hero-original`; versão 1.0.22, código 20029.
- Trailer ocupa a mesma caixa da arte; fonte quadrada/vertical não encolhe
  mais a caixa. Cover uniforme e overscan 1,35 como no original.
- Mesmas paradas lineares do degradê do original, com cor de fundo em vez
  da foto parada. Crossfade de 480 ms só após primeiro quadro real.
- Teste isolado de geometria e shader/compositor GLES real passou, comparando
  273.920 canais de pixels com os colorStops do Compose.
- Preservados os layouts, fileiras, correção de rolagem, ausência de frames
  e preferências de áudio/legendas; nenhum segundo decoder ou blur adicionado.
- [Comparativo e limites](./docs/trailer-original/COMPARATIVO-1.0.22.md).
- APK compilado e assinado com a chave oficial; publicado no canal do aplicativo.
  Atualizador da 1.0.21 conferido com JSON público e hash do APK.
- [Versão 1.0.22](https://github.com/allanlribeiro/StreamMax/releases/tag/v1.0.22) e
  [registro de validação](./docs/trailer-original/VALIDACAO-1.0.22.json).
- Homologação visual e FPS na TCL P7K ainda pendentes.

## Continuidade — rolagem da Home (27/10/2026)

- O usuário autorizou aplicar a correção se o teste isolado fosse bem-sucedido.
  A baseline reproduziu velocidade antiga bloqueando o pré-carregamento; o
  candidato passou antes de alterar a Home.
- A 1.0.21, código 20028, conclui a mola das fileiras que perderam foco, usa a
  coluna lembrada e encerra o trabalho residual após assentarem.
- Testes com foco, geometria e molas reais: trocas rápidas, retorno, Ver tudo,
  fileiras vazias, Herói dividido, Palco, expansão, toque, animações reduzidas,
  30/60/120 Hz simulados e intervalo longo. ASan/UBSan aprovados.
- Alteração concentrada na atualização horizontal. Layout, trailer, degradê e
  retirada dos frames preservados. Cache/GPU/texto aguardam testes próprios.
- Branch `fix/streammax-home-row-settle`; APK compilado e assinado, publicado no canal
  oficial do aplicativo. O atualizador da 1.0.20 foi validado com o JSON público.
- [Versão 1.0.21](https://github.com/allanlribeiro/StreamMax/releases/tag/v1.0.21) e
  [registro de validação](./docs/home-fluidez/VALIDACAO-1.0.21.json).
- Medição de FPS e teste físico na TCL P7K ainda pendentes.
- Notas: [StreamMax 1.0.21](./RELEASE-STREAMMAX-1.0.21.md).

## Continuidade — retirada dos frames (27/10/2026)

- O usuário informou que a 1.0.19 não passou no teste e cancelou a ideia dos frames.
  Tratar o recurso como encerrado.
- Versão 1.0.20, código Android 20027, em `fix/streammax-remove-seek-previews`:
  retirada a integração da fonte e o painel de miniaturas; a barra usa o avanço normal.
- Ajustes antigos ativados não reabilitam miniaturas. Cadastro de addons, contas,
  perfis, VOD IPTV e demais funções seguem a base 1.0.18.
- Testes locais: preferências antigas, avanço, layout, VOD, trailers, OTA e isolamento
  de pacote. APK compilado, assinado e publicado no canal oficial do aplicativo.
- [Atualização 1.0.20 publicada](https://github.com/allanlribeiro/StreamMax/releases/tag/v1.0.20); atualizador da 1.0.19 validado com o JSON público. [Verificações](./docs/sem-frames/VALIDACAO.json).
- Notas: [StreamMax 1.0.20](./RELEASE-STREAMMAX-1.0.20.md).

## Histórico — miniaturas da fonte na 1.0.19 (cancelado pelo usuário)

- Projeto ativo: `legacy-source/`, Android `com.streammax.tv`, baseado no APK
  estável StreamMax **1.0.18**. O Kotlin na raiz é referência histórica.
- Branch: `feat/streammax-source-thumbnails`; implementação do APK em
  `d5b2368942aeb0f44f9458616c78fe5363072910`, versão **1.0.19**, código **20026**.
- O usuário escolheu usar miniaturas publicadas pela fonte. WebVTT declarado
  pelo addon e HLS VOD com trilha de imagens usam o painel existente do player.
- Seekr está fora do ar; a nova opção não depende de chave nem consulta essa API.
- Testes locais passaram: parsers, ASan/UBSan, HTTP real com duas origens,
  cancelamento, recortes RGB, faixa de três quadros e liberação da folha ao
  encerrar o avanço. Teste físico na TCL e fontes reais ainda pendente.
- Detalhes, formatos e limites: [MINIATURAS-DA-FONTE.md](./MINIATURAS-DA-FONTE.md).
- APK 1.0.19 compilado e assinado; [publicado no canal de atualização do aplicativo](https://github.com/allanlribeiro/StreamMax/releases/tag/v1.0.19). O atualizador instalado na 1.0.18 foi validado com o JSON público. [Registro da validação](./docs/miniaturas-da-fonte/VALIDACAO.json).

## Diretriz de migração para Nuvio Native Legacy (17/10/2026)

**Prioridade máxima:** desenvolver o StreamMax em cima da implementação Android de [iqui27/nuvio-native-legacy](https://github.com/iqui27/nuvio-native-legacy/tree/master/android), e não continuar usando o Nuvio original como base do novo APK. O código atual na branch `dev` fica preservado como referência/backup até haver um build funcional da nova base.

### Backlog futuro — Smart TVs Samsung (pedido de 18/10/2026)

- Preparar posteriormente uma distribuicao **StreamMax para Samsung Tizen**, separada do APK Android.
- Aproveitar, quando chegar a etapa, o suporte Tizen que ja existe na base nativa Legacy (`.tpk` e, conforme compatibilidade, `.wgt`), revisando nome, identidade visual, login, controle remoto, reproducao e atualizacoes.
- **Nao implementar, compilar ou publicar a versao Samsung nesta fase.** O pedido e para o futuro.
- Prioridades atuais: corrigir as artes do Continuar Assistindo; aproximar o fluxo e a tela de reproducao do Nuvio original; reduzir a demora do inicio de filmes e episodios sem perder fluidez e desempenho.

### Backlog futuro — visual de TV ao vivo (pedido de 18/10/2026)

- Redesenhar, em uma versão futura, a interface de TV ao vivo para a TCL P7K: guia EPG cinematográfico, logos e categorias de canais, lista navegável pelo controle e mini-player durante a escolha de canal.
- Preservar o motor IPTV já existente (Xtream, Stalker e fontes dos addons) e o fluxo de controle remoto; separar apresentação visual de reprodução para não prejudicar o tempo de troca de canal.
- **Somente backlog:** não modificar a tela de TV ao vivo, motor IPTV, guia ou PiP nesta etapa. Primeiro concluir e validar abertura/troca de fontes, continuidade e player VOD na TCL.

### Requisitos aprovados, na ordem de trabalho

1. Base Android do Legacy, preservando sua arquitetura nativa e fluidez.
2. Identidade StreamMax: nome próprio, instalação lado a lado com Nuvio e Legacy, visual azul cinematográfico e logo aprovada (requer arquivo final).
3. Português brasileiro (`pt-BR`) como idioma inicial, respeitando escolha posterior do usuário.
4. TV ao vivo por IPTV, aproveitando os recursos existentes no Legacy.
5. Home com **Continuar Assistindo** prioritário e opção de trailer automático no destaque.
6. **Execução e reprodução dos conteúdos no estilo do Nuvio original:** reproduzir a experiência de abrir filmes e séries, escolher fontes, retomar e usar controles do player original, sem descartar o núcleo nativo do Legacy.
7. Imagens estáticas, sem GIFs, com foco em TCL P7K e TX6 com 2 GB RAM.
8. Preservar perfis, addons, catálogos e controle remoto. **Validar login QR e login por e-mail/senha antes da compilação final**, com verificações estáticas e testes de autenticação disponíveis; a autenticação real de ponta a ponta depende do backend e do dispositivo. Não incluir credenciais reais em testes de CI.
9. **Validar a autenticação antes da compilação final do APK:** revisar login por e-mail/senha, QR Code, persistência da sessão e sincronização de perfis; testar com backend funcional quando possível. O login por e-mail/senha deve funcionar como alternativa ao QR Code. Não declarar autenticação funcional apenas com análise estática.
10. Build Android no GitHub Actions **somente após implementação e revisão das alterações**. Não gerar APK intermediário.

### Mapeamento técnico inicial (análise, não implementação concluída)

- **Base nativa Legacy:** `iqui27/nuvio-native-legacy` — `src/app.c` coordena o aplicativo; `src/player.c`/`src/player.h` implementam o fluxo de reprodução, e `android/app/src/main/java/space/nuvio/nativelegacy/NvPlayer.kt` usa Media3 ExoPlayer como player Android.
- **Idioma:** `src/ajustes.c` e `src/idiomaauto.h`; a escolha manual tem prioridade. O script em `migration/prepare-legacy.sh` aplica o padrão PT-BR à cópia do Legacy, ainda sem execução e sem validação.
- **TV ao vivo:** `src/xtream.c`, `src/stalker.c`, `src/guia.c`; preservar cadastro, listas e guia. A presença dos módulos no código não prova que a reprodução IPTV funciona após a migração.
- **Nuvio original:** usa arquitetura Android Kotlin/Compose e Media3 diferente. Comparar as jornadas **selecionar título → detalhes → escolher fonte → iniciar reprodução → retomar → legenda/áudio → próximo episódio**; adaptar o comportamento ao Legacy, não copiar diretamente a implementação Kotlin.
- **Critérios de aceite do player:** filme e episódio iniciam corretamente; fontes selecionáveis; posição de retomada respeitada; idiomas/legendas configuráveis; reprodução e navegação por controle remoto estáveis; IPTV continua separado de VOD e funcional.
- **Ainda pendente:** incorporar a árvore do Legacy à estrutura definitiva do fork, aplicar marca no núcleo nativo, definir paleta, logo, home e CI; compilar e testar somente no fim.

### Porta de qualidade obrigatória: login (antes de gerar APK)

- O Legacy atual possui em `src/login.c` a opção de **e-mail e senha**, chamando `sessao_login_email(email, senha)`, além do caminho QR `sessao_login_comecar()`.
- Verificar no código: ambos os fluxos aparecem na interface, entrada de e-mail e senha é funcional, tratamento de erros e dados sensíveis é seguro, persistência/renovação da sessão existe e retorno à tela principal é correto.
- Executar testes de autenticação independentes que não precisem de APK. **Não afirmar login funcionando apenas pela inspeção estática.**
- Caso não exista ambiente backend de teste, marcar o teste real de credenciais e QR como bloqueado, sem pedir senha do usuário nem gravar tokens em GitHub.
- Só autorizar o APK final após revisão de autenticação; o teste real na TV ainda precisará ser feito após a compilação.

### Critério obrigatório de autenticação antes do APK final

- O Legacy já contém login por **e-mail e senha**, além de QR Code, em `src/login.c` (#216). Preservar ambas as opções no StreamMax, com e-mail e senha como alternativa funcional.
- Antes da compilação final, inspecionar `src/login.c`, `src/sessao.c`, dependências de rede e configurações de servidor. Executar testes de código/protocolo disponíveis, sem inserir senhas, tokens ou contas reais no repositório.
- **Não declarar login funcionando** apenas por revisão estática: autenticação real precisa de backend acessível e teste integrado com conta de teste. A compilação do APK pode permanecer bloqueada até essa validação, conforme a decisão do usuário.
- Critérios de aceite: entrar com e-mail/senha, identificar mensagens de erro com credenciais inválidas, persistir sessão, sair e entrar novamente, testar QR se disponível e confirmar sincronização de perfil.
- Se a verificação ponta a ponta não puder ser feita antes do APK, informar a limitação explicitamente e decidir com o usuário como proceder, sem alegar validação que não ocorreu.

### Notas técnicas e limites

- O repositório Legacy contém `android/` e uma implementação nativa C/SDL/GLES; não é um fork Kotlin/Compose que possa substituir diretamente o código Android anterior.
- Migrar em branch separada e verificar dependências/build do Legacy antes de mexer na branch `dev`.
- A compilação anterior do StreamMax usa o código original e **não** constitui APK Legacy.
- Verificar no código e no dispositivo a presença e compatibilidade de cada recurso; não registrar como implementado antes disso.


Documento de referência para desenvolvimento no GitHub/Codex. Atualizado em 17/10/2026.

## Objetivo

Criar e manter um aplicativo Android TV independente, chamado **StreamMax**, derivado de um projeto Android TV open source. Preservar a capacidade de reprodução e navegação, mas adotar uma identidade visual própria e adequada a televisores e TV boxes com pouca memória RAM.

## Decisões de produto e design

- Nome definitivo: **StreamMax** (evitar "Nuvio" na marca exibida).
- Direção visual aprovada: **azul cinematográfico** — preto/azul-marinho profundo, azul elétrico/ciano e discretos acentos violetas.
- Logo de referência aprovada: triângulo de play em formato de fita azul/ciano com leve violeta, sobre fundo escuro sem borda azul; tipografia futurista com **STREAM** claro/prata e **MAX** azul. A arte final aprovada precisa ser fornecida como arquivo-fonte antes de adicioná-la ao repositório.
- Interface limpa, minimalista, focada em navegação por controle remoto, com imagens estáticas. **Evitar GIFs** e efeitos contínuos pesados.
- A fileira **Continuar Assistindo** é prioritária na Home.
- Desejo de manter a opção de **trailer no destaque**, desde que não prejudique a fluidez.
- Atenção especial ao desempenho em aparelhos com cerca de **2 GB de RAM**.

## Requisitos funcionais

1. O StreamMax deve poder ser instalado **ao lado do aplicativo original**, sem substituir sua instalação. Antes de distribuir APK, revisar o `applicationId`, deep links, autoridades e comportamento de atualização.
2. Manter a navegação por controle remoto, reprodução, catálogos e perfis estáveis.
3. Verificar o fluxo **login por QR Code** de ponta a ponta: geração, URL do código, aprovação no celular, polling, troca por sessão e sincronização de perfil.
4. Nunca embutir senhas ou chaves privadas no repositório. Configurar URLs e chaves públicas necessárias à autenticação pelo ambiente de build; credenciais sensíveis devem ficar em GitHub Secrets.
5. Gerar APKs reproduzíveis via GitHub Actions e publicá-los como artefatos para testes no Android TV.

## Estado verificado em 17/10/2026 (após PR #2)

- Repositório: `allanlribeiro/StreamMax`, branch padrão `dev`.
- PR #2 incorporada na branch `dev` (commit `5305e114`): nome de exibição StreamMax e identificador independente no flavor `full`.
- Flavor `full`: `applicationId = "com.streammax.tv"`; a build `fullDebug` usa o sufixo `.debug`. Isso permite instalar o APK de testes ao lado do aplicativo original, sujeito a testes no dispositivo.
- GitHub Actions **PR Full Debug Build**, execução [#39059592307](https://github.com/allanlribeiro/StreamMax/actions/runs/39059592307), concluída com sucesso. A execução disponibilizou artefato `app-full-universal-debug.apk` (~155 MB). Testes unitários do updater e compilação foram executados; isso **não comprova** abertura na TV nem funcionamento do login.
- A configuração de backend do QR Code no workflow é opcional: utiliza a variável `NUVIO_SUPABASE_URL` e o secret `NUVIO_SUPABASE_ANON_KEY`, ou propriedades locais previamente supridas. O êxito do workflow não confirma que essas opções estavam preenchidas, nem que o QR Code funciona.
- A arte final do logo aprovada ainda precisa ser incorporada a partir do arquivo fornecido. Pendentes: teste real da instalação e abertura, autenticação por QR Code, navegação e desempenho na TCL e na TX6.

## Próximas tarefas recomendadas (ordem)

1. Confirmar build verde no Actions e a presença de APKs como artefatos.
2. Separar o identificador de pacote do StreamMax e testar instalação ao lado do app original.
3. Aplicar o nome StreamMax e os recursos de identidade visual aprovados.
4. Validar QR login com ambiente de backend corretamente configurado, sem expor credenciais.
5. Ajustar Continuar Assistindo; testar controle remoto e desempenho.
6. Rever reprodução automática do trailer no destaque e realizar testes reais em dispositivos Android TV.

## Critérios de qualidade

Cada alteração deve ter escopo definido, build/teste quando possível e anotação do que foi ou não verificado. Não alterar mecanismos de reprodução/autenticação sem necessidade. Se uma etapa depender de teste na TV, marcar como **pendente**, nunca como concluída.

Este documento descreve decisões do fork; não é documentação oficial do projeto upstream.

## Feedback TCL P7K — 18/10/2026 (teste real do usuário)

- **Confirmado na TV:** todas as artes de “Continuar assistindo” apareceram; preservar o fallback de imagem.
- **Problema reproduzido:** início de filmes/episódios ainda lento e seleção automática da primeira fonte indesejada.
- **Regra desejada:** perguntar a fonte na primeira reprodução, persistir a escolha do título/perfil e continuar automaticamente nos próximos episódios quando houver fonte equivalente. Se não houver correspondência, permitir nova escolha; não trocar sem avisar uma escolha manual recém-feita.
- **Visual:** reproduzir o *estilo de interface* do player do Nuvio original, com marca e acentos StreamMax. Não substituir cegamente o motor nativo do Legacy por classes Kotlin: preservar desempenho e compatibilidade com TCL.
- **Marca:** não mostrar “Nuvio” nos textos da interface. Preservar endpoints, IDs, pacotes e chaves de sincronização necessárias à conta existente.
- **Alterações preparadas em `streammax/tcl-player-source-branding-1018`:** padrão de escolha manual ativado em instalações novas (preferências gravadas mantidas); substituição do nome no fluxo central de desenho de textos; teste C isolado. **Pendente:** refazer desenho do OSD/player, auditar imagens, caminhos que não passam pelo desenho de texto, compilar APK e confirmar em TCL.

## Player nativo — aproximação visual do Nuvio original (18/10/2026)

- **Código alterado na PR #13:** `legacy-source/src/player.c` passa a renderizar botões redondos fixos com foco branco e ícone preto, na ordem Play / Próximo / Legendas / Áudio / Fontes / Episódios / Proporção / Reiniciar. Nas séries, Próximo aparece quando aplicável; os últimos botões preservam ações úteis do Legacy sem impor um menu novo.
- Barra de progresso: variante `plrui_barra_original`, com tamanho realçado ao focar sem cabeça circular flutuante. Preserva marcadores de capítulo, cor azul StreamMax e buffer.
- Fundo do vídeo: degradês superior/inferior aproximados do player Compose original, reutilizando o shader com dither e sem novos buffers de GPU.
- Preservadas as logos dos títulos sobre os controles, retorno textual quando não há arte e indicação do episódio. Preservados motor nativo, HDR, legendas, fonte selecionada, tempo de reprodução, retomada e navegação por controle.
- Arquitetura: o player oficial em `app/src/main/java/com/nuvio/tv/ui/screens/player/PlayerScreen.kt` (branch `dev`) é Kotlin/Compose, enquanto o Legacy roda SDL/GLES em C. A implementação **adapta o visual** e não substitui motor; não declarar que ambos são tecnicamente idênticos.
- **Validações:** testes estáticos de paridade visual e geometria; compilação Linux nativa em CI proposta sem gerar APK. Revisão visual na TCL e desempenho ainda necessários para homologar.
- **Nenhum APK novo** deve ser disparado enquanto a adaptação visual não estiver revisada e aprovada para teste.

## Fluxo da primeira fonte — compatibilidade com instalacoes antigas (18/10/2026)

- O projeto mudou o **padrao** de escolha manual em novas instalacoes, mas isto **nao atualiza** o `escolherFonteManual 1` que ja ficou salvo nos testes com Legacy.
- A migracao Android de `legacy-source/src/ajustes.c` ativa uma vez a opcao **Escolher a fonte ao reproduzir** tambem nos aparelhos existentes, registrando `streammax-primeira-fonte-1018.txt` no armazenamento local. Se o usuario alterar a opcao depois, a migracao **nao** desfaz a escolha no proximo inicio.
- O fluxo existente de `app.c` abre a folha de fontes se nao houver preferencia manual valida. A escolha fica em `fontepref` por titulo/perfil e pode ser reutilizada nos episodios seguintes quando o addon disponibiliza correspondencia; sem correspondencia, volta a perguntar. Nenhum link expira silenciosamente sendo tratado como uma identidade eterna.
- TV ao vivo continua independente do seletor manual VOD. Nao modificar o motor de canais.
- **Verificacao prevista:** contratos estaticos de carregamento/gravacao, preferencia manual, retomada e ponto ao trocar de fonte; testar na TCL com conta real (filme novo, episodio seguinte, fonte que sumiu, retorno pelo Continuar assistindo, configuracao desligada manualmente).
- **Estado:** codigo proposto em branch separada, sem APK novo; a homologacao funcional e visual do player original na TCL continua pendente. Android 12 e a plataforma alvo, sem atualizar a TV para Android 14 nesta fase.

## Regressões da reprodução — base integrada e testes (18/10/2026)

- A PR #13 (OSD próximo ao player original) foi mesclada em `streammax/legacy-migration`. A PR #14 passa a partir dessa **base integrada**, com diff limitado à migração da primeira fonte, ao seletor parcial, ao contrato de regressão e aos testes sem APK.
- Corrigido `autoParcialPronto()`: quando a opção manual está ativa, usar apenas `fontepref_escolher_manual`, e não a memória do automático. O cache agora invalida também quando muda o título, perfil/configuração ou revisão das preferências, evitando seleção parcial obsoleta.
- Regra de primeira escolha VOD isolada em `streammax_first_source.h` e exercitada em teste C para casos de primeira reprodução, mesma fonte no próximo episódio, desaparecimento da fonte, lista vazia e opção automática escolhida pelo usuário.
- CI de UI/player passa a rodar também na PR de correção, com compilação nativa Linux e regressões; **não gera APK**.
- Ainda não aprovado em dispositivo: o teste com vídeo real na TCL P7K, áudio/legendas, continuidade de episódios, foco do controle remoto, HDR e tempos de carregamento depende de APK de validação posterior. Não chamar o player de homologado antes desses testes.

## Reproducao baseada no Nuvio original — primeira migracao funcional (18/10/2026)

- A PR #14 de escolha inicial e memoria da fonte foi integrada em `streammax/legacy-migration` com testes de fluxo e compilacao Linux aprovados; o proximo trabalho e o **player**, nao TV ao vivo ou Android 14.
- Comparacao real no codigo: o original `PlayerRuntimeControllerInitialization.kt` usa Media3 ExoPlayer, com caminho `DefaultLoadControl.Builder()`. O Legacy Android ja reproduz por Media3 ExoPlayer em `NvPlayer.kt` (por SurfaceView). Trocar para o player original exige migrar **comportamento, OSD e tratamento de sessoes**, nao instalar simplesmente outra biblioteca de video.
- Etapa da PR #15: no VOD com buffer automatico (preset 0) e sem preparo, usar a janela 15–45 s inspirada em `NuvioExoPlayerPerformanceHelper` e os limiares stock Media3 para inicio/rebuffer. Preservar os tetos de bytes dependentes da RAM da TCL. Presets de buffer explicitos e a instancia invisivel de preparo conservam os limiares Legacy.
- Aceleracao esperada: menor espera imposta pelo buffer apos uma fonte valida; **nao** resolve atrasos de consulta a addon ou disponibilidade de link. Monitorar travamentos/rebuffer em fontes lentas e reverter a politica se necessario.
- Teste estatico em `migration/check-player-original-engine.py` e workflow sem APK; falta compilacao Android com Media3/NDK e teste de duracao ate o primeiro quadro com a TCL.
- Criterios da migracao completa **antes do proximo APK**: abertura manual de primeira fonte, continuidade de episodios com mesma fonte quando disponivel, retomada sem voltar ao inicio ao trocar de fonte, OSD e foco proximos do Nuvio original com logo de titulo e acento azul StreamMax, idioma/legendas, 4K HDR10+/Dolby Vision, controle de erros, troca de faixa e fluxo de retorno usando controle remoto.
- Manter compatibilidade Android 12 da TCL P7K; Android 14 e Tizen fora desta etapa.

- Avanco funcional adicional no mesmo player Media3: no VOD com preset Auto e sem preparo invisivel, reter 1,5 s de back-buffer a partir de keyframe, espelhando o `DefaultLoadControl.Builder().setBackBuffer(1_500, true)` do original. O teto de memoria/bytes da TCL nao foi aumentado. Preparo invisivel e preset manual continuam sem essa retencao.
- Telemetria para comparacao em aparelho: log do primeiro quadro (`loadMs`, ja presente) e novos logs de rebuffer apenas **apos** primeiro quadro (`ocorrencias`, `ultimaMs`, `totalMs`). Pausa voluntaria encerra a contagem de rebuffer. Sao medidas da instancia Media3, nao incluem tempo de busca de fontes nos addons.
- Risco e criterio de aceitacao: back-buffer pode disputar memoria com streams 4K de bitrate alto; medir HDR, memoria, inicio, pequenos retrocessos e travamentos na TCL Android 12. Se rebuffer piorar, ajustar a retencao antes de distribuir. Sem Android APK novo nesta etapa.
- Clique curto na fileira de episodios (Android): se ja existe escolha manual persistida para a serie no perfil, seguir o fluxo normal de reproducao em vez de reabrir a folha de fontes em todo episodio. O roteador so toca automaticamente quando a lista atual confirma a identidade dessa fonte; se nao casar, a folha de fontes reaparece. Primeira escolha continua obrigatoria e menu de fontes por pressao longa continua explicito. Outras plataformas preservam regra anterior.
- Testes de unidade do seletor e verificacao do elo `detail.c` → `app.c` incluidos no job da PR #15. Confirmar na TCL P7K clique em episodios (primeiro, segundo e fonte ausente) antes de fechar a PR.


## Controle de escopo sem retrabalho — player (18/10/2026)

**Regra de trabalho:** preservar a versão experimental testada e aprovada na TCL. Não reiniciar a migração, não refazer etapas integradas e não misturar TV ao vivo/Android 14/Tizen com a PR #15. Antes de propor alteração, procurar contrato, implementação e regressão já existentes. Só alterar diferenças funcionais específicas comprovadas na comparação com o Nuvio original.

**Já implementado; não refazer por padrão:**
- PRs #13 e #14 integradas: OSD inspirado no original com marca StreamMax, escolha inicial manual, migração das preferências antigas e memória de fonte por título/perfil. Permanecem pendentes as provas completas em dispositivo.
- PR #15: Media3 ExoPlayer já é o motor de Android; ajuste stock 15–45 s sem preset manual, limiares de início/rebuffer, proteção do primeiro quadro/pausa, telemetria, retomada, adoção de sessão preparada e pré-consulta apenas de metadados quando houver escolha manual.
- Fluxos atuais de áudio, legenda, HDR, Dolby Vision, foco do D-pad e fontes: preservar até identificar falha concreta, sem substituir o player por Compose ou reiniciar código.
- Correção na PR #15: seek descarta cues de legendas atrasadas e limpa a sobreposição; contrato estático adicionado. Execução de verificações de código #39153689712 concluída com sucesso. Compilação Android #39153689886 foi observada em andamento; não deduzir aprovação dela.

**Só faltam validação de equivalência funcional e eventuais correções pontuais comprovadas:**
- Conferir em base real o tempo do primeiro quadro, troca e retomada de fonte, continuidade e seleção de episódios, pausa/Back, buffer e recuperação após erro, faixas de áudio/legenda, 4K e HDR/Dolby Vision na TCL P7K com Android 12.
- CI/testes estáticos comprovam presença de contratos, não execução completa com vídeos reais. Não chamar a reprodução de 100% equivalente ou homologada sem prova prática.
- Pedido expresso do proprietário: **não enviar APK experimental para novos testes**. Oferecer apenas versão candidata final depois de revisão funcional e verificações automáticas; a homologação de 100% na TCL ainda necessitará avaliação real do proprietário.

**Próxima tarefa de código:** auditar somente diferenças ainda não cobertas pelo código/testes em PR #15 e corrigir regressões demonstráveis. Evitar alterar arquivos fora do player/contratos, exceto documentação técnica de controle de escopo.

### Estado da lacuna de legendas (sem retrabalho, 18/10/2026)

- **Implementado na PR #15, aguardando build/teste TCL:** legendas gráficas que Media3 entrega em Cue.bitmap (por exemplo PGS/VobSub compatíveis com o decoder) agora passam a um SubtitleView Canvas exclusivo para bitmaps, criado sob demanda na camada já existente do vídeo. Não há outra Surface de vídeo nem alteração da camada SDL do OSD.
- **Preservado:** caminho de legendas de texto em nativeLegenda, seleção/desativação de faixas, atraso positivo para cues, foco do controle remoto, buffer/Media3, escolha manual da fonte, perfil e continuidade de episódios.
- **Proteções novas:** descartar cues antigos ao trocar faixa ou buscar outro ponto, remover canvas ao liberar a sessão, acompanhar o retângulo de vídeo mesmo com letterbox/recorte. A dependência media3-ui tem a mesma versão 1.8.0 do motor.
- **Ainda pendente:** confirmar que a camada aparece na TCL durante reprodução real e que formatos gráficos específicos são decodificados; avaliar fidelidade completa de estilo/animações ASS em relação ao AssSubtitleView do Nuvio original. Não confundir implementação e aprovação em dispositivo.
- A checagem estática migration/check-player-original-engine.py cobre o novo caminho e a compilação Android determina se o código Kotlin/Media3 integra sem erros. A exigência de não enviar APK intermediário permanece.

## Feedback real TCL — motor de vídeo e seletor visual (18/10/2026)

- **Confirmado pelo usuário na TCL:** reprodução de conteúdo 4K, Dolby Vision e HDR está funcionando no APK gerado pelo GitHub Actions #39156897487. Não inferir desta validação que todas as demais funções de reprodução e faixas estão aprovadas.
- **Diferença visual comprovada:** a escolha de legenda ainda surgia como painel Legacy no canto superior direito, ligado à ilha do relógio. A adaptação anterior de barra e botões do player **não incluía** esse painel; não descrever a estética inteira como equivalente ao Nuvio original.
- **Correção nesta PR #15:** a folha de legendas e a aba de estilo pedem cartão independente ancorado embaixo à esquerda, preservando seleção, foco, faixas e o mesmo motor Media3; a folha de áudio permanece temporariamente como estava. Testes estáticos adicionados.
- **Escopo ainda pendente de fidelidade visual:** as colunas específicas de idiomas, opções e estilo de `SubtitleSelectionOverlay.kt` (Compose) ainda não foram totalmente reproduzidas no painel nativo SDL/C. Não confundir mudança de posição com paridade completa.
- **Pendente:** compilação e validação do novo painel na TCL — abertura via botão Legendas, setas/OK/Voltar, textos sem corte e uso da aba Estilo; repetir o teste 4K/DV/HDR após a mudança antes de declarar a interface homologada.

## Evolucao do seletor de legendas — coluna de idiomas

- **Solicitacao confirmada pelo usuario:** manter o motor de reproducao 4K / HDR / Dolby Vision aprovado na TCL, mas aproximar a interface de legendas do Nuvio original.
- **Implementado nesta PR #15:** painel ancorado a esquerda e ampliado somente para legendas, com coluna de idiomas navegavel via D-pad e ponteiro, lista de opcoes independente, OK no idioma filtrando a lista sem iniciar/recarregar qualquer legenda; a aba de Estilo segue existente e acessivel a direita.
- **Compatibilidade intencional:** audio da folha anterior preservado, filtros de fontes, posicao do video e parser de legenda fora do escopo.
- **Pendencias visuais:** Estilo ainda usa aba propria, nao e uma terceira coluna simultanea identica a Compose; testar na TCL abertura, idiomas, navegacao ESQUERDA/DIREITA, OK/Voltar, muitas linguas e textos compridos.
- **Solicitacao subsequente, ainda nao implementada:** melhorar o filtro de audio das fontes para reconhecer dublagem PT-BR tambem com etiquetas genericas de portugues. A rotina atual esta em `streams.c` (`temAudioPreferido`, `audioDeclaradoEm`, `faixaIdiomaDoNome`); preservar distincao entre PT-BR confirmado, PT-PT explicito e PT generico, sem afirmar origem brasileira por chute de legenda, URL ou titulo.
