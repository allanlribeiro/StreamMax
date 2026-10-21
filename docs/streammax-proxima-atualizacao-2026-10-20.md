# StreamMax — próxima atualização (trabalho em andamento)

Base técnica: `release/streammax-1.0.8-ota-keyboard`. Branch de trabalho:
`streammax/next-pt-regional-iptv-20261020`.
**Não publicar por atualização automática até concluir os testes na TCL P7K.**
O nome da futura versão será decidido depois da validação; não reduzir a numeração
ao número 1.0.7, que já está ocupado no histórico de releases.

## Estado da implementação na branch de desenvolvimento

### 1. Abertura cinematográfica
- Código Android TV implementado em `StreamMaxAberturaVideo.kt` e ligado
  ao ciclo de vida de `NuvioActivity.kt`. Exibe o vídeo somente na primeira
  abertura TV, se `assets/streammax_abertura.mp4` estiver empacotado.
- Player nativo Android (MediaPlayer) em camada temporária, sem GIF, timeout
  de 6,5 s, sem segundo Media3, com liberação de recursos quando termina,
  falha ou app sai para background. Splash estático preservado como fallback.
- `tools/android.sh` empacota opcionalmente
  `art/marcas/streammax_abertura.mp4`; Gradle não comprime o MP4.
- **Pendente essencial:** o arquivo aprovado
  `StreamMax_Abertura_Grave_Metalica_720p.mp4` (1280 × 720, H.264/AAC,
  4,5 s, ~410 KB) está disponível na Biblioteca ChatGPT, mas NÃO foi
  transferido ao repositório/estágio de build. APKs atuais ainda exibem
  fallback estático. Não afirmar que o vídeo está funcionando em aparelho
  enquanto esse arquivo não for empacotado e testado.

### 2. Xtream TV ao vivo
- A base continua usando HLS/TS alternativos (`xtream_formatos`),
  watchdog de congelamento (`livestall.h`) e diagnóstico (`livetvdiag.c`).
- Novos eventos `[xtream-stall]` diferenciam EOF, fonte seca, rebuffer
  e posição congelada, sem salvar usuário/senha/URL do provedor.
- Testes C de regressão validam pausa, reconexão progressiva e cancelamento
  quando o buffer cresce. **Sem conclusão empírica de correção dos
  travamentos na TCL P7K**; necessita logs e teste real antes de ajustar buffer.

### 3. VOD separado dentro da área IPTV
- `xtream.c/h`: APIs sob demanda para categorias, filmes,
  séries e episódios; IDs e extensões validados antes de montar URLs.
- `streammax_iptv_vod.c/h`: três áreas visíveis no guia Android TV
  (Canais ao vivo / Filmes IPTV / Séries IPTV); navegação pelo D-pad
  mesmo sem teclas coloridas. Listas carregadas em worker assíncrono
  com orçamento fixo de 48 categorias, 96 títulos ou 96 episódios por
  consulta; primeiro acesso baixa só categorias, filmes/séries só quando
  uma categoria é explicitamente escolhida.
- `guia.c`: tabs com foco, volta para a TV ao vivo pelo Back, suspende
  prévia automática durante o VOD.
- `app.c`: caminho direto de abertura de filmes/episódios Xtream pelo
  player já existente (sem pedir fontes ao Torrentio/addons), identidades
  `xtvod:`/`xtepisode:`.
- `player.c`: bloqueia publicação de progresso, Trakt, Simkl,
  atividade social e Continuar Assistindo para os itens do provedor.
  São `CatItem` temporários para o player, não publicados como novas
  fileiras na Home nem mesclados com a biblioteca de filmes/séries TMDB.
- **Limites conhecidos:** 96 títulos por categoria (ainda sem paginação),
  lista de episódios de até 96, sem autoplay de próximo episódio;
  sem indicador de progresso personalizado IPTV. Validar navegação, vídeo,
  troca de tela, memória, privacidade e reconexões na TCL P7K antes do release.

### 4. Legendas de português por região
- `linguas.c/h` e `legendasui.c` separam PT-BR, PT-PT e Português
  sem região confirmada no painel de seleção. Preferência regional explícita
  não seleciona automaticamente a variante oposta. Áudio mantém casamento
  genérico por família.
- Testes C em `test_streammax_subtitle_regions.c` e validação de
  compilação sintática executados. Teste com arquivos reais na TV pendente.

### CI e distribuição
- `StreamMax next - idiomas e IPTV` testa linguagens, categoria VOD,
  watchdog de canais e sintaxe nativa de `streammax_iptv_vod.c`,
  `guia.c`, `app.c`, `player.c` (SDL/GLES em Linux).
- `StreamMax APK experimental` está habilitado para branch de
  desenvolvimento sem OTA. Não converter builds experimentais em release,
  nem publicar atualização automática durante o trabalho.
- PR #32 continua em rascunho; o template de PR exige issue aprovada
  e a funcionalidade GitHub Issues deste repositório está desabilitada.

## Gates de release

1. Testes C / regressões CI.
2. Renderização correta com controle remoto e arquivo audiovisual exato.
3. VOD UI navegável, com playback e sem troca para catálogo principal.
4. Teste do Xtream com reconexão, estabilidade e uso de memória na TCL P7K.
5. Build Android TV assinada e teste de atualização sem perder perfis.
6. Só então, merge em branch de release e disponibilização no atualizador.

A implementação é incremental; **compilação sintática e testes unitários não
significam APK pronto**.
