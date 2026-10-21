# Xtream VOD — contrato de paginação e segurança de memória

## Achados verificados no código (21/10/2026)
- `xtream_vod_listar_categoria` em `legacy-source/src/xtream.c` usa `get_vod_streams&category_id=...` ou `get_series&category_id=...`, sem parâmetros de página.
- `chamarSt` baixa o corpo completo antes do parse; `n < cap` limita somente os objetos copiados, **não** o JSON recebido.
- A UI `streammax_iptv_vod.c` mantém até 96 títulos, 48 categorias e 96 episódios. Excedentes ficam inacessíveis.
- A rede Xtream usa `travaRede` para serializar requisições, protegendo provedores contra rajadas. Não paralelizar chamadas por página sem avaliar o rate limit.

## Estratégia de implementação (sem inventar suporte do provedor)
1. Criar testes com resposta sintética contendo >96 títulos, IDs válidos/inválidos e categorias distintas; verificar ordem, deduplicação, continuidade e ausência de itens perdidos.
2. Definir interface de consulta com cursor/offset **local**, mantendo geração de sessão e limite de itens desenhados; preservar seleção e foco ao solicitar mais.
3. Somente usar `page`/`limit` remotos quando o painel confirmar suporte. Os endpoints Xtream comuns não garantem paginação; não presumir que parâmetros ignorados funcionam.
4. Para provedores sem paginação: evitar repetir download integral a cada avanço. Investigar parser incremental com armazenamento temporário limitado (por categoria e perfil), limite explícito de bytes, cancelamento e limpeza ao trocar de lista/perfil. Não manter JSON inteiro e todos os pôsteres na RAM.
5. Exibir indicação de resultados incompletos até a navegação total ser implementada. Não apresentar “96 de 96” como catálogo completo se o retorno tiver mais.
6. Revalidar categorias (>48) e episódios (>96) com o mesmo princípio, sem alterar a Home principal.

## Critérios de aceite
- Navegação pelo controle remoto percorre títulos acima do 96º sem saltos/duplicatas.
- Troca de aba/categoria invalida resposta antiga; consultas não congelam a UI.
- Canal ao vivo não dispara carregamento de VOD.
- Sem URL, token, usuário ou senha em logs, arquivos de diagnóstico ou testes.
- Testes sintéticos e CI aprovados; teste real de memória e navegação na TCL P7K antes do merge.
- Preservar 1.0.8 e não gerar APK como “pronto” antes da validação.

**Estado:** especificação técnica de implementação; nenhuma mudança de paginação em runtime foi realizada neste commit.
