# StreamMax — correções de reprodução e fontes (18/10/2026)

## Erros confirmados por teste na TCL

1. Filme ou episódio **nunca assistido** inicia automaticamente, sem pedir a primeira fonte.
2. **Torrentio TB** fornece muitas fontes (captura do StreamMax mostra 282), mas algumas dubladas que aparecem no Nuvio original não chegam à lista do StreamMax, inclusive com o filtro de idioma desativado.

## Comportamento esperado

- **Sem histórico de reprodução:** abrir seletor de fontes e aguardar seleção explícita; não pré-selecionar e iniciar o primeiro resultado.
- **Com histórico de reprodução:** retomar na última fonte quando ainda disponível, mantendo a posição; caso contrário abrir seletor.
- **Escolha de fonte:** continuar permitindo troca manual a qualquer momento.
- **Filtros:** quando desativados, exibir todas as fontes válidas recebidas. Não descartar dublagens pela heurística de nome antes da apresentação.

## Diagnóstico técnico necessário

- Localizar o ponto que decide iniciar o player e a distinção entre conteúdo novo e retomada; testar histórico inexistente, histórico de 0 segundos, progresso real e fonte indisponível.
- Comparar chamadas de stream do Torrentio TB (mesmo conteúdo, mesmo ID, mesmo addon/configuração) entre Nuvio original e StreamMax.
- Instrumentar contagens: resposta HTTP, parsing, deduplicação, classificação de áudio/idioma, filtro e UI. Não incluir segredos ou URLs com tokens em logs.
- Conferir identificação de `Dublado`, `PT-BR`, `Portuguese`, `Dual Audio` e metadados de áudio reais, sem presumir que título do arquivo sempre revela o idioma.

## Critérios de aceite

- [ ] Conteúdo inédito espera escolha da fonte antes de iniciar.
- [ ] Conteúdo iniciado continua automaticamente apenas quando a fonte anterior é válida.
- [ ] Fonte anterior ausente abre seletor.
- [ ] Filtro desligado não elimina fontes dubladas na lista.
- [ ] Comparação Nuvio original vs. StreamMax feita com mesmo título, ID e configuração.
- [ ] Testado em APK instalado na TCL.

**Status:** requisitos e casos de teste documentados; implementação e validação pendentes.
