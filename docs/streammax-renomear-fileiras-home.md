# StreamMax — Renomear fileiras da Home (filmes e séries)

**Pedido corrigido:** 20/10/2026. **Estado:** especificação registrada; implementação e testes pendentes.

## Escopo correto
Permitir que o usuário altere **o título exibido das fileiras/listas de filmes e séries na tela inicial (Home)** do StreamMax — por exemplo, trocar o título de uma fileira "Ação" por "Filmes de ação favoritos" — diretamente na TV, usando o controle remoto.

**Não se trata de renomear fontes ou listas IPTV M3U/Xtream.** A proposta anterior de renomeação de listas IPTV foi um mal-entendido e foi descartada.

## Experiência desejada
1. Na Home ou no gerenciador de fileiras da Home, escolher a fileira desejada de filmes ou séries.
2. Disponibilizar a ação **Renomear** em um menu de opções acessível pelo controle remoto, preservando a navegação normal e o botão OK para abrir conteúdos.
3. Abrir um campo com o título atual, editável com o teclado da TV, e ações **Salvar** e **Cancelar**.
4. Ao salvar, alterar imediatamente o título daquela fileira na Home.
5. Manter o nome escolhido ao fechar/reabrir o aplicativo e ao atualizar os dados da fileira.

## Restrições e critérios de aceite
- Renomear **somente o título de exibição da fileira**, sem alterar os filmes/séries, gêneros, ordem dos cards, filtros, URL de catálogo, add-ons, metadados ou reprodução.
- Usar identificador estável da fileira, não seu título original, para persistir a personalização.
- Se a fileira for alimentada por um catálogo externo, manter a personalização local mesmo quando o catálogo atualizar o título de origem; não modificar a fonte remota.
- Suportar fileiras personalizadas/geradas, e avaliar separadamente quais fileiras de sistema poderão receber título editável, conforme estrutura real da Home.
- Salvar por perfil, se as configurações da Home forem específicas de perfil; respeitar o mecanismo existente de sincronização, quando houver.
- Impedir nome vazio ou apenas espaços; **Cancelar** não deve gravar alteração.
- Preservar fluidez em Android TV e TV Box com 2 GB de RAM; evitar recomputações durante navegação.
- Validar com o controle remoto: renomear fileira de filmes, fileira de séries, cancelar, reiniciar, recarregar catálogo e alternar perfis quando aplicável.

## Planejamento
Avaliar inclusão na StreamMax 1.0.7 junto aos demais ajustes, sem afirmar que a função já foi implementada. Os pontos de interface, armazenamento e testes ainda precisam ser desenvolvidos.
