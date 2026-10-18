# StreamMax — guia de continuidade do projeto

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
