# StreamMax — guia de continuidade do projeto

## Situação em 18/10/2026 — StreamMax Legacy (atual)

- **Código em desenvolvimento:** branch `streammax/legacy-migration`, baseada no Nuvio Native Legacy (C/SDL/GLES para Android). A branch `dev` segue como referência e hospeda o workflow do APK experimental.
- **APK disponível:** GitHub Actions [execução 39089781586](https://github.com/allanlribeiro/StreamMax/actions/runs/39089781586), concluída com sucesso em 18/10, artefato `StreamMax-APK-EXPERIMENTAL` (~53 MB), com validade prevista até 25/10. Trata-se de build experimental, **não** de validação de funcionamento em dispositivo.
- **Conta existente:** interface do Legacy agora usa somente e-mail e senha; a configuração temporária de build aponta para o backend público `https://api.nuvio.tv` e sua chave pública publishable já divulgada pelo projeto original. O usuário não precisa criar um projeto Supabase próprio para esse teste. **Ainda não foi confirmado** login real na conta, renovação do token, perfis e sincronização dos addons/configurações.
- **Rede:** o log da execução 39089781586 confirma ausência de `libcurl.so` nas duas arquiteturas; o código dispõe de fallback para requisições simples via `HttpURLConnection` no Android, mas imagens, reprodução e fluxos avançados precisam de teste real. Não afirmar que playback está corrigido apenas porque o job compilou.
- **Proteção nova de CI:** commit [49e1e19](https://github.com/allanlribeiro/StreamMax/commit/49e1e195017967d32546a19a19f008289929f8e6) faz a verificação pós-build da URL e chave pública incorporadas às duas `libmain.so`, e do package name `com.streammax.tv.strictdebug`, antes de publicar o artefato. A aprovação dessa nova execução ainda deve ser confirmada.
- **Próxima validação na TCL P7K:** instalar ao lado do Nuvio original e Legacy, abrir o StreamMax, autenticar com o mesmo e-mail/senha da conta Nuvio, conferir carregamento dos perfis e catálogos, capas, abertura de filme e episódio, áudio/legendas e controle remoto. Priorizar eventuais falhas de autenticação/reprodução antes de trailer automático e refinamentos visuais.
- **Preferências que permanecem:** identidade azul cinematográfica, Português (Brasil), Continuar Assistindo prioritário, imagens estáticas sem GIF, fluidez na TCL P7K e uso da TX6 como dispositivo secundário de teste.

**Atenção:** o registro histórico abaixo menciona QR Code porque documenta requisitos anteriores. A decisão atual para o APK experimental é **login exclusivo por e-mail e senha**, preservando como meta acessar a mesma conta e dados do Nuvio.

---

## Diretriz de migração para Nuvio Native Legacy (17/10/2026)

**Prioridade máxima:** desenvolver o StreamMax em cima da implementação Android de [iqui27/nuvio-native-legacy](https://github.com/iqui27/nuvio-native-legacy/tree/master/android), e não continuar usando o Nuvio original como base do novo APK. O código atual na branch `dev` fica preservado como referência/backup até haver um build funcional da nova base.

### Requisitos aprovados, na ordem de trabalho

1. Base Android do Legacy, preservando sua arquitetura nativa e fluidez.
2. Identidade StreamMax: nome próprio, instalação lado a lado com Nuvio e Legacy, visual azul cinematográfico e logo previamente aprovada (pendente fornecimento do arquivo final).
3. Português brasileiro (`pt-BR`) como idioma inicial, respeitando escolha posterior do usuário.
4. TV ao vivo via IPTV integrada, aproveitando os recursos existentes no Legacy.
5. Home com **Continuar Assistindo** prioritário e opção de trailer automático no destaque.
6. Imagens estáticas, sem GIFs, com foco em TCL P7K e TX6 com 2 GB RAM.
7. Preservar perfis, addons, catálogos, player, navegação por controle remoto e login QR (validar em dispositivo).
8. Build de APK Android com GitHub Actions e teste real de instalação e abertura.

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
