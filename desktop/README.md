# StreamMax para Windows — porte experimental

Referência Android: `feature/streammax-111-static-s-logo` (StreamMax 1.0.11).
Desenvolvimento isolado: `feature/streammax-windows-desktop`.

## Objetivo inalterado
Portar **a experiência do Android**, mantendo tela inicial, ordem dos catálogos, destaques, login, detalhes, player e histórico. Alterar somente a interação necessária a mouse/teclado e janelas Windows.

## Implementado no código desta ramificação
- Estrutura de janela Electron independente do Android TV, com F11.
- Tela inicial **em desenvolvimento**: hero superior (texto à esquerda e arte à direita) e linhas horizontais. Usa apenas metadados retornados pelos catálogos reais; não cria títulos fictícios. Foco por mouse/teclado, setas horizontais, rolagem e carregamento em lotes.
- Integração inicial de login de dispositivo via Supabase, com abertura da página de autorização no navegador.
- Sessão protegida por `safeStorage`, renovação de tokens, restauração com validação remota e logout seguro.
- Consulta somente-leitura dos addons da conta para o **perfil principal (1)**, via `get_sync_owner` e tabela `addons`.
- Busca de manifestos e páginas de catálogos dos addons usando requisições separadas, sem encaminhar tokens Supabase a terceiros.
- Detalhes dos títulos consultados do addon de origem: descrição, metadados e **lista real de episódios**, com filtros de temporada e seleção explícita do episódio antes de buscar fontes. A pesquisa de fontes acontece conforme cada addon responde, sem aguardar todos os demais.
- Addons que fornecem apenas streams são reconhecidos mesmo sem apresentar catálogos.
- Testes de código, autenticação, visualização de metadados e catálogos com respostas simuladas.

## Player mpv externo — primeira etapa
- Fontes com URL direta HTTP(S) podem ser abertas em **janela externa do mpv**, usando uma conexão IPC Windows com nome aleatório. A URL assinada não aparece na linha de comando nem é devolvida ao JavaScript da interface.
- Na tela de detalhes, selecione uma fonte direta e clique em **Reproduzir no mpv**. Na primeira utilização, escolha um `mpv.exe` confiável com **Configurar mpv.exe**. O caminho é salvo em `userData/mpv-settings.json`; não guardamos URL nem token nesse arquivo.
- O mpv **não vem embutido** no instalador atual. Precisa ser instalado/obtido separadamente pelo usuário até que haja uma solução de empacotamento com licenças e dependências verificadas.
- Funções iniciadas: pausa/retomada, salto de ±10s, alternância de faixa de áudio, legenda e visibilidade de legenda. Metadados reais de faixas alimentam seletores separados de áudio e legenda; os idiomas pt-BR, pt-PT e português sem variante conhecida são diferenciados. A sessão continua em uma janela própria do mpv; a integração de progresso, legendas pt-BR/pt-PT, configurações de áudio e reprodução integrada ainda não está concluída.
- Apenas streams com URL direta são aceitos nessa primeira etapa; fontes torrent/infoHash e protocolos não suportados são listados como indisponíveis para reprodução direta.

## O que ainda não está pronto
**Não há equivalência funcional com a versão Android.** A tela inicial é uma implementação parcial da organização Modern, ainda não foi comparada visualmente pixel a pixel. Estão pendentes: seleção/sincronização de outros perfis e preferências de layout/hero, Continuar Assistindo, trailers, busca completa, detalhes completos, seleção de fontes avançada, legendas pt-BR/pt-PT, áudio configurável, player integrado com codecs/formatos adequados, preferências, atualização e testes ponta a ponta.

A tela de detalhes **já busca metadados reais do addon de origem e lista fontes**, mas ainda não é uma reprodução funcional: falta portar a seleção final da fonte e o player Windows. Para séries, só pesquisa fontes depois que um episódio válido for escolhido; não tenta reproduzir o identificador da série. Se os metadados de episódios estiverem indisponíveis, informa a limitação sem inventar episódios.

## Configuração de desenvolvimento
Instale Node.js LTS. Em `desktop/`, use `npm install`, `npm test`, `npm start`.
Para consultar a mesma conta que o Android, é necessário configurar **por um método local seguro**, sem fazer commit de credenciais, as variáveis públicas e de servidor:
- `STREAMMAX_SUPABASE_URL`: URL HTTPS do backend usado pelo Android;
- `STREAMMAX_SUPABASE_PUBLIC_KEY`: chave **publicável** (nunca service role);
- `STREAMMAX_LOGIN_REDIRECT_URL`: endereço HTTPS compatível de autorização de dispositivo no backend.

Não envie segredos por chat nem em issues. As credenciais de sessão devem permanecer no armazenamento protegido do Windows. A versão Windows ainda não implementa a descoberta automática da configuração de servidor do Android.

## Ícone e instalador
A arte S aprovada deve estar em `desktop/assets/streammax-s.png` (quadrada, 256 px ou maior); o workflow converte para ICO multirresolução. Sem essa arte, o workflow **não** gera instalador para não usar ícone diferente. Instaladores experimentais não são assinados.

A versão 0.1.0 já executada pelo usuário é anterior a este porte de interface. **Atualizações de código não alteram automaticamente esse executável.**

## Licenças
Preservar o licenciamento GPLv3 do projeto original e as licenças dos componentes externos. Não distribuir segredos embutidos. Validar atualizações e reprodução diretamente em Windows antes de publicar release.
