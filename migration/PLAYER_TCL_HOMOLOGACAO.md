# StreamMax — homologação funcional do player na TCL P7K

Estado: **pendente de teste real**. Este roteiro não é laudo de aprovação.

## Base técnica verificada

- A PR #15 implementa a migração comportamental do Media3 em `legacy-source/android/app/src/main/java/space/nuvio/nativelegacy/NvPlayer.kt`.
- O player oficial usa Media3 em `app/src/main/java/com/nuvio/tv/ui/screens/player/PlayerRuntimeControllerInitialization.kt`.
- O Legacy mantém renderização/OSD em C/SDL. Não confundir paridade visual com equivalência de sessão, buffering e decoder.
- Alvo imediato: TCL P7K Android 12, sem migrar para Android 14, TV ao vivo ou Tizen.
- Não alterar URLs, provedor, conta, credenciais, cache ou configurações de usuário para realizar testes.

## Matriz de aceite (executar em APK identificada por commit SHA)

Registrar para cada caso: APK/commit, hora, título e episódio, identificador de addon/fonte sem expor URLs privadas, mídia, duração até primeiro quadro, pausas, erros do logcat, resultado e evidência visual.

| ID | Cenário | Aprovação exigida |
| --- | --- | --- |
| P01 | Filme nunca visto, sem fonte preferida | Mostra seleção de fontes ANTES de abrir qualquer mídia; não inicia sozinho |
| P02 | Filme com fonte escolhida manualmente | Retomada pode usar fonte válida do mesmo título/perfil sem perguntar de novo; se ausente, mostra lista |
| P03 | Episódio seguinte com mesma fonte disponível | Continuidade automática respeita a preferência e o idioma |
| P04 | Episódio seguinte sem fonte equivalente | Mostra lista, sem selecionar fonte não confirmada |
| P05 | Troca de fonte com filme em andamento | Preserva o ponto de reprodução, não troca de episódio nem inicia do zero |
| P06 | Retomar de Continuar Assistindo | Preserva progresso, arte existente e controles do player |
| P07 | Pausa manual antes do primeiro quadro | Não retoma sozinho por watchdog, READY ou adoção de sessão preparada |
| P08 | Alternar para segundo plano e voltar | Não toca em background nem altera intenção de pausa |
| P09 | Fonte lenta / buffering | Mede tempo até primeiro quadro e rebuffer; não fica preso em READY sem imagem |
| P10 | Seleção de áudio e legendas | Troca faixas corretamente; sem cues antigos ao trocar legenda ou atraso |
| P11 | HLS, MP4, fonte Torrentio TB/debrid autorizada | Comportamentos validados separadamente; ausências de resultados de addon não são culpa presumida do motor |
| P12 | SDR, HDR10+ e Dolby Vision, quando suportados pela fonte e TV | Imagem correta, áudio correto, sem recriação contínua da Surface |
| P13 | Controle remoto D-pad, Back, pausa, seek | Foco e retorno corretos; sem controles presos ou ANR |
| P14 | Abertura repetida e troca entre episódios | Sem vazamento de Surface/player, áudio duplicado ou travamento |
| P15 | Preset de buffer manual | Preserva valor definido pelo usuário e os limites de RAM |
| P16 | Instalação por cima de APK assinada previamente | Confirmar mesma assinatura e package; preservar dados. APK debug com assinatura diferente não é teste válido de atualização direta |

## Medições

- Medir primeiro quadro por log de `onRenderedFirstFrame`; registrar tempo real com fonte idêntica em pelo menos 3 tentativas e comparar com o Nuvio original na mesma TCL e mesma rede.
- Registrar `rebufferCount` e `rebufferTotalMs`, separando buffering inicial do buffering após o primeiro quadro.
- Conferir logs de `originalStock`, `inicio`, `rebuffer`, `backBuffer` e teto em bytes; não declarar rapidez apenas com constantes.
- Anotar codec, resolução, HDR/DV e a reprodução após troca de fonte; testar stream de acesso permitido.
- Salvar logs sem tokens de debrid, URL assinada, chave, cookie ou dados pessoais.

## Gates para fechar PR #15

1. Testes de contrato do player e compilação Android aprovados no **mesmo SHA** da versão testada.
2. P01–P10 e P13–P16 com evidência na TCL; P11/P12 nos formatos disponíveis ou explicitamente marcados como não testados.
3. Nenhuma regressão crítica em login, início de filme, fonte manual, legenda, áudio ou retomada.
4. Corrigir e retestar qualquer falha observada antes da aprovação.
5. Somente então solicitar revisão e integração; uma compilação verde, sozinha, NÃO constitui homologação na TCL.

Registro de execução: **nenhum desses testes está sendo declarado realizado neste documento**.
