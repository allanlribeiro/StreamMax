# StreamMax Legacy 1.0.14 — addons e abertura

## Origem correta

Repositório: allanlribeiro/StreamMax. Base desta alteração: `5d8ab8116c7ec717a9bfaf7fc53c4b6e3f211863`, branch `feature/streammax-addons-cadastro-tv-20261023`.

A versão pública aprovada é 1.0.11. A branch de cadastro contém a tentativa 1.0.12 e a correção 1.0.13 que moveu o botão para a primeira linha. Esta revisão preserva essa correção e usa 1.0.14 / Android code 20021, pacote `com.streammax.tv`, com a assinatura oficial já configurada no CI.

O projeto local `/workspace/Streamax` e o APK `app.streamax.tv.debug` pertencem à outra base Kotlin/Compose. Não são a origem desta alteração nem uma atualização da linha Legacy instalada na TCL.

## Problemas encontrados e correções

1. O cadastro persistia o endereço antes de consultar o manifesto. Agora a consulta acontece num worker, com prazo e tamanho limitados, sem bloquear a navegação.
2. Uma página HTML de configuração ou erro podia entrar como addon. Agora são exigidos JSON completo, campos de identificação e recursos; `configurationRequired=true` orienta a configurar no provedor.
3. Entrada com espaços finais era recusada, apesar de o normalizador geral aceitá-la. O cadastro agora remove apenas espaços externos e preserva a caixa de caminhos e tokens.
4. A confirmação não capturava a visita ao perfil. Agora o pedido fica vinculado à sessão de perfil desde a abertura do teclado; o resultado só é gravado sob a guarda dessa sessão. Cancelar ou trocar perfil invalida o resultado pendente.
5. Um endereço já presente na conta podia ser salvo como local antes de a lista recusar a duplicata. A lista é conferida antes da persistência; falha na inclusão reverte a nova entrada local.
6. A inspeção de host poderia incluir a query de URLs sem caminho. O delimitador agora exclui `?` e `#`, protegendo configurações que levam tokens.
7. Os avisos de progresso e conclusão usam a mesma identidade, substituindo o estado anterior sem deixar “verificando” sobre um resultado já concluído.

O botão Adicionar continua sendo a primeira linha, inclusive com a lista vazia ou muitos addons. A atualização da descoberta permanece agrupada na saída da tela.

## Nova abertura

PNG original desta conversa, estático, com símbolo azul/ciano e nome STREAMMAX. O Android e o motor SDL usam o mesmo arquivo. A proporção é preservada e a imagem é centralizada. A textura usa o cache existente; não há vídeo, GIF, processamento da imagem por quadro ou tempo mínimo adicional. A transição para os perfis foi preservada.

O login nesta base já é exclusivo por e-mail e senha. Não foi adicionado QR Code nem alterado o contrato de autenticação.

## Verificação e limites

Testes host do normalizador, armazenamento por conta/perfil, estados, remoção, deduplicação e navegação existente. Novos testes executam o parser real e o worker real com transporte controlado: sucesso, HTTP recusado, transporte, HTML, configuração obrigatória, cancelamento e mudança de perfil. Gates de isolamento Android, autenticação e contratos de reprodução permanecem.

O CI produz somente candidato de teste e confere o certificado contra a 1.0.11. A compilação não prova login com conta real, navegação física na TCL, reprodução, comportamento do teclado Android ou qualidade de vídeo. Essas verificações continuam pendentes antes de considerar atualização estável/OTA. Nenhuma senha ou URL pessoal é necessária ou incluída nos testes.
