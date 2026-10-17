# StreamMax — guia de continuidade do projeto

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
