# StreamMax Legacy — reprodução e Torrentio TB

## Defeitos confirmados na TCL P7K

- Conteúdo nunca assistido inicia reprodução automaticamente sem abrir seletor de fontes.
- Fontes dubladas do Torrentio TB aparecem no Nuvio original mas não no StreamMax, inclusive sem filtro de dublagem. Captura do StreamMax exibe 282 fontes desse addon; contagem alta não comprova equivalência.

## Regras de reprodução

1. Sem progresso/histórico real: mostrar seletor de fontes e aguardar confirmação do usuário antes de iniciar o vídeo.
2. Com progresso e fonte anterior disponível: retomar automaticamente na fonte e posição anteriores.
3. Se fonte anterior não existir: abrir seletor; não iniciar fonte substituta automaticamente.
4. Não confundir reprodução automática do trailer no destaque com autoplay de filme/episódio.
5. Preservar seleção manual de áudio, legenda e fontes e desempenho da TCL.

## Investigação Torrentio TB

1. Comparar a URL/configuração do mesmo addon e o mesmo ID de filme/episódio nas duas aplicações sem expor credenciais.
2. Comparar número de entradas por estágio: resposta, parse, deduplicação, filtro por idioma, apresentação.
3. Revisar classificação de nomes PT-BR, Portuguese, Dublado, Dual Audio sem presumir que toda fonte contém marcador explícito.
4. Quando filtro estiver desligado, nenhuma fonte válida deve ser removida por idioma.

## Testes de aceite pendentes

- [ ] Novo filme solicita fonte antes de reprodução.
- [ ] Primeiro episódio novo solicita fonte antes de reprodução.
- [ ] Continuação retoma fonte e posição anteriores quando existentes.
- [ ] Fonte antiga ausente solicita nova seleção.
- [ ] Fontes Torrentio TB comparadas com Nuvio original sob configurações idênticas.
- [ ] APK compilado e validado na TCL.

**Status:** investigação e implementação ainda pendentes; não confundir critérios documentados com correção concluída.
