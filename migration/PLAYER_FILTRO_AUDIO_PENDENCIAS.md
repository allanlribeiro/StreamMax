# StreamMax — seletor de idioma de áudio: estado e pendências

Estado em 19/10/2026: **implementado no código; validação final em CI e na TCL P7K ainda pendente. Não liberar APK final.**

## Confirmado por inspeção do código da PR #15
- Chip de áudio na folha de fontes alterna Todos, Português, Inglês e Dual Audio, sem depender do idioma da interface.
- `temAudioPreferido()` prioriza o áudio declarado em `audioIdiomas` e usa o nome do release apenas quando não há declaração; legendas não são prova de áudio.
- `stream_faixas_texto()` apresenta bandeiras nos idiomas de áudio declarados: 🇧🇷 para português (inclusive pt-PT) e 🇺🇸 para inglês.
- Dual Audio exige dois idiomas distintos declarados ou indicação textual explícita não ambígua.
- `passaChips()` filtra somente a visualização; retornar a Todos não descarta fontes nem addons.
- Quando não há áudio declarado e não se identifica o idioma, `idiomaDa()` retorna `Áudio não identificado` (commit `5c71399`).
- Teste de regressão do rótulo desconhecido e precedência da declaração adicionado a `streammax-source-ptbr.sh` (commit `6f06611`).

## Ainda falta para finalizar
1. Conferir resultados das verificações automáticas após `6f06611` e a compilação nativa.
2. Na TCL P7K, usar o controle remoto para percorrer os quatro modos e confirmar metadados desconhecidos e áudio diferente das legendas.
3. Confirmar retorno de todas as fontes com Todos, inclusive Torrentio/TorBox, além de fonte manual, retomada, continuidade e pré-carregamento.
4. Revisar acabamento estético e testar a reprodução real, a pausa e a troca de faixas na TCL.
5. Gerar novo APK apenas depois das correções e verificações, conforme combinado.

Essas verificações de código não equivalem a homologação com vídeo e login reais na TV.
