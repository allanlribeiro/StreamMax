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
- Funções iniciadas: pausa/retomada, salto de ±10s, alternância de faixa de áudio, legenda e visibilidade de legenda. Metadados reais de faixas alimentam seletores separados de áudio e legenda; os idiomas pt-BR, pt-PT e português sem variante conhecida são diferenciados. A sessão continua em uma janela própria do mpv; o progresso local está implementado no código, mas ainda faltam testes reais no Windows, sincronização com a TV, preferências completas de áudio/legendas e reprodução integrada.
- Apenas streams com URL direta são aceitos nessa primeira etapa; fontes torrent/infoHash e protocolos não suportados são listados como indisponíveis para reprodução direta.

- **Continuar Assistindo local:** mpv agora informa tempo, duração e buffer para o Electron; posição registrada em **milissegundos**, em armazenamento criptografado pelo Windows, seguindo limites do Android (iniciado 2%, concluído 90%). Os episódios são diferenciados por série, temporada e episódio; a retomada usa a posição do vídeo exato ao abrir o mpv novamente. A aba Continuar Assistindo exibe apenas o progresso salvo neste computador; ela permite pesquisar novamente as fontes antes de reproduzir. Uma prévia **somente de leitura** do histórico remoto do Android está disponível na aba Continuar Assistindo, via `sync_pull_watch_progress`, com limite de registros e filtragem. O backend não fornece nome/arte nesse RPC: a prévia exibe identificadores, não títulos inventados. Agora os itens do histórico da TV podem abrir a busca de fontes no Windows e a retomada usa **primeiro o histórico local e, na ausência dele, a posição autenticada da TV**. A consulta é iniciada após o login em paralelo ao carregamento dos catálogos para reduzir espera. A posição nova continua sendo salva apenas no Windows. Mesclagem persistente, sincronização bidirecional e escrita de progresso na TV **ainda não estão implementadas**. A saída da conta limpa o histórico local para evitar que outra conta veja esses dados.

## Limites da retomada entre dispositivos
- A consulta de histórico da TV usa o RPC existente `sync_pull_watch_progress`. O recurso só funciona quando a conta Windows está autenticada no mesmo backend do Android e o perfil principal é utilizado.
- O retorno remoto fornece identificadores e tempos, não inclui título nem pôster: a prévia exibe identificadores, sem arte inventada.
- A retomada remota é um fallback: uma posição válida já salva neste computador tem prioridade. Em caso de erro de rede, a reprodução pode iniciar do começo.
- O progresso gerado no Windows permanece no armazenamento criptografado local e **não é enviado ao Android** nesta etapa.
- Os testes isolados usam respostas simuladas. A validação do fluxo com backend real e mpv no Windows permanece pendente.

## O que ainda não está pronto
**Não há equivalência funcional com a versão Android.** A tela inicial é uma implementação parcial da organização Modern, ainda não foi comparada visualmente pixel a pixel. Estão pendentes: seleção/sincronização de outros perfis e preferências de layout/hero, mesclagem e escrita remota do Continuar Assistindo, trailers, busca completa, detalhes completos, seleção de fontes avançada, legendas pt-BR/pt-PT, áudio configurável, player integrado com codecs/formatos adequados, preferências, atualização e testes ponta a ponta.

A tela de detalhes **já busca metadados reais do addon de origem e lista fontes**, mas ainda não é uma reprodução funcional: falta portar a seleção final da fonte e o player Windows. Para séries, só pesquisa fontes depois que um episódio válido for escolhido; não tenta reproduzir o identificador da série. Se os metadados de episódios estiverem indisponíveis, informa a limitação sem inventar episódios.

## Configuração de desenvolvimento
Instale Node.js LTS. Em `desktop/`, use `npm install`, `npm test`, `npm start`.
Para consultar a mesma conta que o Android, o aplicativo agora permite **configurar a conexão na primeira abertura** se não houver variáveis do Windows. Essa opção registra apenas o endpoint HTTPS, a chave pública/anon e a URL de autorização, reinicia o StreamMax e limpa sessões antigas caso a conexão seja alterada. Para desenvolvedores, também é possível usar as variáveis:
- `STREAMMAX_SUPABASE_URL`: URL HTTPS do backend usado pelo Android;
- `STREAMMAX_SUPABASE_PUBLIC_KEY`: chave **publicável** (nunca service role);
- `STREAMMAX_LOGIN_REDIRECT_URL`: endereço HTTPS compatível de autorização de dispositivo no backend.

Não envie segredos por chat nem em issues. As credenciais de sessão devem permanecer no armazenamento protegido do Windows. A edição Windows ainda não descobre automaticamente o servidor configurado na TV. As variáveis têm prioridade sobre o arquivo local de configuração; nunca forneça `service_role`, chaves privadas ou tokens de sessão.

## Compilar versão de teste usando o código existente

No Windows, abra PowerShell na pasta `desktop` e execute:

```powershell
.\scripts\build-test.ps1 -LogoPath "C:\caminho\streammax-s.png"
```

O script copia a arte **S aprovada**, valida a suíte de testes, abre o Electron em modo de verificação de interface, gera o ícone e compila o instalador unsigned na pasta `desktop\dist`. Ele **interrompe a compilação se algum teste falhar**. Requer Node.js, Python e conexão à internet para instalar as dependências; não modifica o código Android.

Sem a logo oficial no repositório, o pipeline do GitHub ainda executa testes e smoke test, mas não publica instalador. A arte PNG original tem prioridade sobre qualquer JPEG de prévia.

## Ícone e instalador
A arte S aprovada deve estar em `desktop/assets/streammax-s.png` (quadrada, 256 px ou maior); o workflow converte para ICO multirresolução. Sem essa arte, o workflow **não** gera instalador para não usar ícone diferente. Instaladores experimentais não são assinados.

A versão 0.1.0 já executada pelo usuário é anterior a este porte de interface. **Atualizações de código não alteram automaticamente esse executável.**

## Licenças
Preservar o licenciamento GPLv3 do projeto original e as licenças dos componentes externos. Não distribuir segredos embutidos. Validar atualizações e reprodução diretamente em Windows antes de publicar release.
