# StreamMax 1.0.9 — diagnóstico de inicialização do player

Base: `release/streammax-1.0.8-ota-keyboard`. Este documento **não implementa** instrumentação, nem altera o buffer.

## O que já existe (não duplicar)
- `legacy-source/android/app/src/main/java/space/nuvio/nativelegacy/ParaleloDataSource.kt`: download paralelo de MKV/MP4 progressivos; primeiro trecho de 512 KiB; segmentos/manifestos HLS/DASH fora dessa via; cache de redirecionamento por sessão.
- `legacy-source/src/streammax_source_audio.h` e `legacy-source/tests/test_streammax_source_audio.c`: classificação de Dual Audio e idiomas explícitos; nunca inferir PT-BR de “Dual Audio” ou de legendas.
- `legacy-source/src/player.c`: interface existente; não redesenhar.

## Próxima alteração pequena e isolada
Instrumentar **somente** a sessão de abertura (com relógio monotônico) nos pontos de entrada existentes, após localizar as funções efetivas:
1. Seleção da fonte até URL resolvida (`source_resolve_ms`).
2. URL resolvida até primeira resposta/byte (`first_byte_ms`).
3. Primeiro byte até player pronto para apresentar (`prepare_ms`).
4. Player pronto até primeiro quadro exibido (`first_frame_ms`).
5. Total da seleção ao primeiro quadro (`startup_total_ms`).

Registrar uma única linha por tentativa/sessão, inclusive erro/cancelamento, sem expor URL, token, credenciais, nome de arquivo ou conteúdo. Não acrescentar logs por frame, não abrir conexões extras, não aumentar buffers e não mudar políticas de retry nesta etapa. Distinguir fonte progressiva, HLS e DASH; não assumir que todos percorrem o mesmo caminho.

## Critérios de aceitação
- Build Android e testes existentes passam.
- Reproduzir e medir na TCL P7K: filme 1080p Torrentio/TorBox, filme 4K, canal Xtream ao vivo, troca de fonte e seek.
- Conferir primeiro quadro, buffering, cancelamento, falha de CDN e retorno ao addon.
- Verificar que o APK não cresceu significativamente e que o player não ficou mais lento.
- Comparar medianas de pelo menos 3 tentativas por cenário com a 1.0.8; nenhum ganho deve ser declarado sem medições.
- Preservar comportamento e layout já aprovados; sem alteração de assinatura ou atualização automática nesta fase.

## Separação de escopo
As quatro prioridades da próxima versão (vídeo de abertura, Xtream ao vivo, VOD IPTV separado e legendas pt-BR/pt-PT) seguem independentes deste diagnóstico. Não duplicar implementações existentes; verificar antes de modificar.

## Verificação de integração (21/10/2026)
Inspeção de `NvPlayer.kt` confirmou que o player principal **já registra** `loadMs` usando `SystemClock.elapsedRealtime() - abriuEm` em `onPlaybackStateChanged`, `onRenderedFirstFrame` e `onPlayerError`. Há também contadores de rebuffer. Portanto, **não adicionar logs equivalentes nem outro cronômetro total em paralelo**. O coletor `PlayerStartupTiming.kt` e os testes são preparatórios; ainda não estão conectados ao player.

Pontos de ligação possíveis, condicionados a conferir ciclo de vida e geração de sessão:
- `SELECTED`: momento em que a abertura da sessão define `abriuEm`; reutilizar o mesmo instante monotônico, não iniciar um segundo relógio divergente.
- `RESOLVED`: somente quando houver callback efetivo de resolução da URL; não confundir `setMediaItem` com resolução de addon.
- `FIRST_BYTE`: distinguir resposta HTTP de primeiro byte de corpo; `Estat.primeiroByteMs` em `ParaleloDataSource.kt` mede até `responseCode` por conexão/redirecionamento e **não** representa necessariamente primeiro byte de mídia ou o tempo da sessão inteira.
- `PREPARED`: primeira transição `STATE_READY` da sessão; não contar rebuffer posterior.
- `FIRST_FRAME`: `onRenderedFirstFrame` da sessão ativa, evitando eventos atrasados após troca de fonte.
- `ERROR/CANCELLED`: finalizar uma única vez e não registrar identificadores/URLs.

Antes de instrumentar, verificar se o `loadMs` existente já satisfaz a medição total; priorizar apenas as lacunas verificadas. Para HLS/DASH, fases indisponíveis devem permanecer nulas, sem inventar durações. Nenhum ganho de desempenho foi demonstrado nesta revisão.
