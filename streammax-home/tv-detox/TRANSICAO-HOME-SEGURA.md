# StreamMax Home 1.0 — transição segura para HOME na TCL P7K

Status: **RC1 experimental**. Não é ainda a versão de distribuição assinada permanentemente.
O pacote `com.streammax.home.rc` é propositalmente separado dos testes anteriores e da
futura distribuição `com.streammax.home`.

## Proteções obrigatórias

- **Nunca** executar `pm disable-user`, `pm uninstall` ou `pm clear` no launcher original
  `com.google.android.apps.tv.launcherx` nem em serviços TCL, antena, HDMI, Bluetooth,
  teclado, Google Play, WebView ou atualizações.
- **Jamais** definir a nova home sem antes identificar e salvar o componente HOME original
  e produzir um comando de retorno com ele.
- Garantir ADB funcional antes e depois da seleção, e preservar scripts e arquivos de backup
  em `Documents/StreamMaxHome-Backups` no computador.
- `StreamMax Home 1.0 RC1` possui `MAIN`+`HOME`+`DEFAULT` e pode aparecer como
  **candidato** de launcher, mas instalar o APK **não muda** a home por si.
- Priorizar seleção pela página `Tela inicial padrão` do próprio StreamMax Home, se
  existir no firmware. Se não aparecer, testar `cmd package set-home-activity` via ADB
  **somente após** a confirmação do componente original e autorização do usuário.
- Anotar antes da mudança: `cmd package resolve-activity --brief -a android.intent.action.MAIN -c android.intent.category.HOME`;
  `cmd package query-activities --brief -a android.intent.action.MAIN -c android.intent.category.HOME`;
  `dumpsys package preferred-activities`.
- No Windows PowerShell, o ADB pode estar em
  `$HOME\Downloads\platform-tools-latest-windows\platform-tools\adb.exe`.
  Porta de depuração sem fio pode mudar após reinício; confirmar `adb devices`.
- O retorno deve selecionar o componente exato original:
  `adb -s IP:PORTA shell cmd package set-home-activity --user 0 COMPONENTE_ORIGINAL`
  (substituir `COMPONENTE_ORIGINAL` pela saída validada, **nunca adivinhar**).
  Verificar com `resolve-activity` depois.
- Depois da troca, testar Home no controle, TV Aberta, lista de entradas, Play Store,
  Configurações, rede, retorno à home após abrir app, botão Voltar e reinício normal.
- Se o TCL interceptar o botão Home, registrar o comportamento e **não desabilitar**
  o Google TV para forçar o atalho.

## Assinatura estável para versão 1.0 de distribuição

GitHub Actions assina normalmente o APK *debug* com uma chave de build temporária.
Compilações diferentes podem usar assinaturas diferentes; por isso não são atualizáveis.
A 1.0 definitiva exige **nova chave de assinatura de propriedade do usuário**, criada
localmente, guardada de forma segura, e configurada em GitHub Actions Secrets.
Não fazer commit de arquivos `.jks`, `.keystore`, senhas, base64 de chave, ou tokens.
Sem a chave, usar apenas RC em pacote separado `com.streammax.home.rc` e não
anunciar suporte a atualizações permanentes. Não reutilizar a chave do app StreamMax.
