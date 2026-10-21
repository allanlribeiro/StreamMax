# StreamMax — próxima atualização (trabalho em andamento)

Base técnica: `release/streammax-1.0.8-ota-keyboard`. Branch de trabalho:
`streammax/next-pt-regional-iptv-20261020`.
**Não publicar por atualização automática até concluir os testes na TCL P7K.**
O nome da futura versão será decidido depois da validação; não reduzir a numeração
ao número 1.0.7, que já está ocupado no histórico de releases.

## 1. Abertura cinematográfica

- Vídeo aprovado pelo usuário: `StreamMax_Abertura_Grave_Metalica_720p.mp4`,
  preservado na Biblioteca do ChatGPT. Duração real: 4,5 segundos;
  resolução 1280 × 720, H.264 + AAC, ~410 KB.
- Áudio: assinatura metálica original, mais grave, aprovada em 20/10/2026.
- O vídeo ainda **não está** dentro do repositório; o desenvolvedor deve
  incorporar exatamente esse arquivo, sem sintetizar uma animação diferente.
- A abertura atual em `legacy-source/src/abertura.c` desenha arte estática
  e impõe teto de 1,5s. Verificar ordem dos quadros e camada Android/SDL
  para tocar vídeo uma só vez; nunca iniciar dois players Media3, não tocar
  ao retornar de background, liberar decoder e áudio ao terminar.
- Se a decodificação falhar ou animações reduzidas estiverem ativadas, usar
  o splash estático atual; não travar o login nem a tela de perfis. Validar
  uso de RAM e tempo de entrada.

## 2. Xtream TV ao vivo

- A base já tem HLS/TS alternativos (`xtream_formatos`), watchdog para
  congelamento (`livestall.h`) e diagnóstico da TV ao vivo (`livetvdiag.c`).
- Esta branch adiciona eventos `[xtream-stall]` sem credenciais para
  diferenciar EOF, falta de segmentos, rebuffer e posição congelada.
- Testes de regressão validam pausa voluntária, reconexão progressiva e
  cancelamento quando o buffer volta a crescer.
- Ainda precisa de registro de reprodução real na TCL P7K para identificar
  causa dos travamentos; não diminuir agressivamente o buffer nem forçar TS
  em todos os provedores sem medida.

## 3. VOD dentro da área IPTV, separado do catálogo normal

- Backend no `legacy-source/src/xtream.c`: leitura sob demanda de categorias,
  filmes (`get_vod_streams`), séries (`get_series`), temporadas e episódios
  (`get_series_info`), montagem segura de URL da reprodução.
- **Pendente:** interface Android TV com **Canais ao vivo**, **Filmes IPTV**
  e **Séries IPTV**; evitar misturar itens com TMDB/addons e Continuar Assistindo
  do catálogo principal. Tela por abas/linhas de baixo custo, preferir
  paginação/limites e carregamento sob demanda; bloquear pré-carregamento
  simultâneo de bibliotecas inteiras do provedor.
- Não imprimir URLs de reprodução de filmes/episódios, pois contêm senha do
  serviço IPTV; não armazená-las no histórico ou telemetria.

## 4. Legendas de português por região

- Implementado: agrupar PT-BR, PT-PT e Português sem região reconhecida
  separadamente no painel de escolha, a partir dos metadados reais.
- Preferência regional explícita não escolhe automaticamente a variante
  contrária. Preferência `pt` genérica continua podendo aceitar ambas.
- A comparação de família de idiomas usada pelo áudio permanece intacta.
- Testes no fluxo puro de linguagem: `test_streammax_subtitle_regions.c`.
  Validação final na TV com arquivos contendo ambos os idiomas pendente.

## Gates de release

1. Testes C / regressões CI.
2. Renderização correta com controle remoto e arquivo audiovisual exato.
3. VOD UI navegável, sem troca para catálogo principal.
4. Teste do Xtream com reconexão, estabilidade e uso de memória na TCL P7K.
5. Build Android TV assinada e teste de atualização sem perder perfis.
6. Só então, merge em branch de release e disponibilização no atualizador.

A implementação é incremental; **compilação sintática e testes unitários não
significam APK pronto**.
