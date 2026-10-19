# StreamMax — pendência funcional: seletor de idioma de áudio

Estado em 19/10/2026: **não concluído; não gerar APK até implementar e validar**.

## Evidência encontrada na PR #15
- `legacy-source/src/streams.c`: `idiomaFiltroAudio()` usa `ling_audio_fonte(idioma_iso(ajustes_idioma()))`, ou seja, deriva o idioma de áudio do idioma da interface.
- O chip `BT_DUB` liga/desliga `soDub`. Não existe nesse caminho seleção independente entre Português, Inglês, Dual Audio e Todos.
- `temAudioPreferido()` respeita `audioIdiomas` formal e só infere PT-BR de nomes/descrições quando não há áudio formal declarado. **Preservar essa precedência**.
- O teste `streammax-source-ptbr.sh` valida classificação de PT-BR; **não comprova seletor de quatro modos**.

## Critérios antes da compilação
1. Separar filtro de **idioma de áudio** da preferência de idioma da interface.
2. Oferecer Todos, Português, Inglês e Dual Audio na tela de fontes, mantendo controle acessível pelo remoto.
3. Mostrar 🇧🇷 para português (incluindo pt-PT) e 🇺🇸 para inglês onde o áudio for identificado; Dual Audio não é sinônimo de dublagem portuguesa.
4. Se o addon não declara áudio e o release não traz indício suficiente, exibir **não identificado**; não inferir idioma a partir de legenda.
5. Filtros devem reduzir apenas a visualização das fontes compatíveis; desativá-los deve restaurar o conjunto original sem excluir Torrentio/TorBox.
6. Não regressar escolha manual da primeira fonte, retomada, continuidade nem pré-carregamento.
7. Testes separados para os quatro modos, flags da grid e metadados ausentes, além dos testes já existentes.

## Regra de liberação
Nenhum APK antes de implementação, validação das verificações sem APK e resolução das pendências restantes da PR #15.
