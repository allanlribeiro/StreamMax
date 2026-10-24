# StreamMax Legacy 1.0.17 — Continuar assistindo no Google TV

Base: 1.0.16 (2995296; ramo técnico 5097640). As alterações preparatórias do YouTube foram canceladas pelo proprietário e NÃO entram nesta versão.

## Implementação

AndroidX TV Provider / WatchNextProgram: publica até 12 títulos recentes do perfil ativo, com capa, título, episódio, posição, duração, horário real da última reprodução e Intent explícito de retorno ao StreamMax. Sem root, troca de launcher, serviço extra, WebView ou novo player. Registros vêm de progresso.c e do catálogo já existente; nenhuma consulta de rede adicional.

A ponte SDL entrega UTF-8 em bytes a um executor serial Android. O loop só examina dados quando há progresso alterado, mudança de contexto ou após 30 segundos. Ao sair para a Home, o checkpoint existente grava o ponto e publica imediatamente. Filtros seguem os limites de início do Watch Next (filmes: 3% ou 2 minutos, o menor; episódios: mais de 2 minutos), conclusão configurada e aproximação de créditos para episódios.

Só o episódio mais recente de cada obra pode aparecer. Conteúdo concluído, oculto ou removido é excluído. Mudança de perfil/conta e tela de login/seleção limpam os cartões anteriores. O publisher só altera registros com seu próprio package_name e prefixo interno. Dispensas na launcher são lembradas e não retornam até nova reprodução. Estado e tarefas são limitados; I/O fica fora do fio de vídeo.

O identificador de perfil publicado é um hash com sal local, sem UUID de conta nem tokens. Somente imagens em hosts públicos conhecidos são enviadas; demais casos usam o banner do app, evitando divulgar URLs privadas ou credenciais de addons. Os links carregam escopo do perfil e só são liberados após login/PIN normais; o código nativo também rejeita links pendentes de uma sessão anterior.

## Validação

Testes Java reais da política de progresso/créditos, URLs, limites, escopos e imagens públicas. Testes C reais de links com troca de sessão enquanto o PIN/login atrasa a entrega. Gates de addons, logo, trailers, autenticação, isolamento e updater; compilação ARM64/ARMv7 com o certificado oficial antes da entrega do APK.

## Limites que precisam de teste na TCL

A integração publica no TV Provider oficial. A Google/TCL controla se o launcher Google TV exibe esses itens no seu Continuar assistindo, a posição da fileira e as políticas de apps/fontes aceitos. Não é garantido apenas porque o insert funciona. Não houve teste físico no aparelho nem certificação Media Actions. IDs privados/incompatíveis com os deeplinks existentes não são exportados; episódios especiais sem temporada positiva abrem o conteúdo pela retomada existente. Capa de fallback pode ser o banner StreamMax.

Referências: https://developer.android.com/training/tv/discovery/watch-next-add-programs e https://developer.android.com/training/tv/discovery/guidelines-app-developers . Versão 1.0.17, Android code 20024, pacote com.streammax.tv. Distribuição pelo atualizador existente, com confirmação do instalador Android.
