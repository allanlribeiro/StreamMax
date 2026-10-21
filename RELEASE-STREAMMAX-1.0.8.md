# StreamMax 1.0.8 — Correção do teclado ao renomear fileiras

Esta é uma atualização corretiva da versão 1.0.7, construída sobre a mesma base Android TV Legacy.

## Correção
- **Renomear fileira:** ao segurar OK em um pôster e selecionar "Renomear fileira", a caixa de texto agora deve abrir e aceitar comandos do controle remoto.
- O teclado de renomeação passou a receber corretamente os eventos de navegação, mostrar a interface e processar confirmação/cancelamento.
- Os nomes personalizados das fileiras de filmes e séries são aplicados à Home sem modificar o catálogo ou o conteúdo da lista.
- Os nomes continuam separados por perfil e persistem após reiniciar o app.

## Mantido sem alterações
- Pôsteres maiores aprovados na 1.0.7.
- Otimização do menu Fontes ao reproduzir em 4K.
- Resolução de vídeo, bitrate, HDR, Dolby Vision, player e fontes.

## Instalação
No StreamMax, abra **Configurações → Atualizar o aplicativo** e confirme a instalação. O sistema Android pode solicitar permissão ou confirmação. Não desinstale a versão anterior.

A compilação automatizada é uma validação inicial: a correção ainda precisa de teste funcional na TCL P7K, especialmente escrever, salvar, cancelar e manter o nome após reabrir.
