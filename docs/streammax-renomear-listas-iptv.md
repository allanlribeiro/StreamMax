# StreamMax — Renomear listas IPTV

**Pedido:** 20/10/2026. **Estado:** especificação registrada; implementação e testes pendentes.

## Objetivo
Permitir que a pessoa usuária atribua um nome personalizado a uma lista IPTV existente (M3U ou Xtream) diretamente na interface Android TV, sem precisar recadastrar a fonte.

## Fluxo de interface
1. Abrir IPTV > Gerenciar listas (ou o menu de gerenciamento equivalente).
2. Selecionar uma lista e escolher a ação **Renomear**.
3. Abrir campo de texto preenchido com nome de exibição atual, utilizável com teclado remoto.
4. **Salvar** novo nome ou **Cancelar** sem mudanças.
5. Atualizar nome exibido na lista e persistir entre reinicializações.

## Restrições e critérios de aceite
- Alterar apenas o nome de exibição, não a URL, credenciais Xtream, IDs internos, dados de favoritos, canais ou conteúdos.
- Não recarregar ou desconectar a lista para efetuar a renomeação.
- Prevenir nomes vazios e descartar mudanças ao cancelar.
- Suportar M3U e Xtream conforme a implementação real de cada provedor.
- Preservar desempenho na TCL P7K e TV Boxes de pouca RAM.
- Não afetar player, qualidade 4K/HDR/Dolby Vision, add-ons ou catálogo principal.
- Se houver sincronização de listas por conta, definir persistência coerente do nome personalizado e não substituir o nome da origem inadvertidamente.
- Validar manualmente: criar lista, renomear, reiniciar app, cancelar, verificar credenciais e canais.

## Integração no planejamento
Avaliar junto da StreamMax 1.0.7. Não confundir esta especificação com implementação concluída. As alterações de UI, armazenamento e testes ainda precisam ser feitas.
