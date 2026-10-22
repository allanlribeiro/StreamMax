# StreamMax 1.0.10 — portabilidade seletiva do Nuvio Legacy 2.0.8

**Base estável:** StreamMax 1.0.9 (APK oficial assinado, `com.streammax.tv`).
**Branch de trabalho:** `feature/streammax-1.0.10-legacy-208`.
**Upstream:** [Nuvio Legacy 2.0.8](https://github.com/iqui27/nuvio-native-legacy/releases/tag/v2.0.8).
**Status:** implementação parcial, sem release, sem alteração do canal OTA público.

## Objetivo
Atualizar estabilidade e desempenho na TCL P7K, sem substituir o player Media3 do
StreamMax, alterar a interface aprovada nem retomar IPTV. Outras Android TVs e
aparelhos de 2 GB de RAM são critérios de compatibilidade.

## Integrado nesta branch (pendente de build Android e teste real)
- **Trim Android em primeiro plano:** porta a guarda do lifecycle do Legacy
  2.0.8 para ignorar níveis de background quando a Activity estiver visível.
  Mantém `NvPlayer.retomarPeloSistema()` e o encerramento da vinheta do StreamMax.
- **Texturas visíveis:** o teto de textura sob pressão nunca cai abaixo dos
  bytes das imagens quentes do quadro atual; evita descarte e recarregamento
  das capas em uso. Adaptação pontual em `tex_cache.c`, sem substituir o arquivo.
- **APKs antigos do atualizador:** limpeza assíncrona somente da pasta privada
  `filesDir/dados/atualizacao`. Remove APKs reconhecidos e já instalados,
  preserva um candidato mais novo e deixa intactos arquivos desconhecidos,
  downloads parciais e demais dados. Preserva verificações de pacote,
  certificado, código de versão e assinatura já existentes no StreamMax.
  O Nuvio upstream usa outro diretório; não copiar o caminho dele literalmente.

## Parcialmente integrado (aguarda validação)
- **Legendas:** porta o reaproveitamento de downloads durante a sessão do
  Legacy 2.0.8 (`legenda.c`). Evita baixar a mesma URL novamente ao mudar de
  faixa, mantém as referências separadas por reprodução e copia os bytes para
  o consumidor sem invalidar o cache. Metadados de provedor e interfaces
  regionais ainda precisam de análise específica.

## Parcialmente integrado: progresso (aguarda validação)
- **Retomada:** checkpoint local a cada 30 segundos efetivos de reprodução;
  sincronização de conta é solicitada em intervalos separados de cinco
  minutos. Eventos de suspensão enviam um checkpoint pelo loop SDL, sem
  bloqueio de armazenamento no fio principal do Android. Avisos de memória
  baixa não forçam sincronização da conta. Mantidos controles de fonte
  principal, duração real, episódio, progresso mínimo e filtro de trailers.
  A persistência em um desligamento forçado ainda exige teste na TCL.

## Próximas etapas (não implementadas)
1. **Legendas:** revisar os metadados do provedor e a interface existente,
   preservando pt-BR/pt-PT e os menus aprovados.
2. **Arte e biblioteca:** fallback TMDB para logos, backdrops em busca e
   sincronização de snapshots, em alterações isoladas.
4. **Desempenho adaptativo:** portar melhorias de `gpunivel.c` somente com
   medições A/B; não piorar o visual azul nem forçar efeitos permanentes.
5. **TCL P7K:** testar 1080p/4K, retorno do player, Home com muitas fileiras,
   seleção de fontes, legendas, suspensão, desligamento e atualização OTA.

## Portas de qualidade
- Executar `python3 legacy-source/tests/streammax-110-check.py` e testes
  de regressão existentes do StreamMax.
- Executar os testes Kotlin `StreamMaxApkCacheTest` em ambiente Android
  configurado. O helper foi isolado do framework e validado também em Kotlin/JVM.
- Só compilar APK de teste depois de passar pela revisão e testes.
- Não alterar `SM_UPDATE_VERSION` para 1.0.10 nem publicar `v1.0.10`
  antes de gerar um APK assinado, confirmar mesmo certificado da 1.0.9 e
  validar pelo menos na TCL.
- **Nenhum trabalho de IPTV ou Tizen nesta versão.**

**Observação:** testes estáticos e unitários não comprovam FPS, tempo de
abertura, imagem HDR ou memória no dispositivo; essa validação continua
pendente.
