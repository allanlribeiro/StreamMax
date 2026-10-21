# RC3 — Teste do botão HOME sem flash do Google TV

## Objetivo
Avaliar experimentalmente se o Google TV da TCL entrega `KEYCODE_HOME` ao AccessibilityService com `flagRequestFilterKeyEvents`. O Android pode reservar essa tecla ao sistema: **não há garantia de interceptação**. A funcionalidade de redirecionamento via evento de janela permanece como fallback e pode mostrar o Google TV rapidamente.

## Precauções
- Não desativar ou desinstalar Google TV, seus serviços, Projectivy ou atualizações de sistema.
- RC3 é aplicativo de teste. Desativar sua acessibilidade para reverter.
- Desligar primeiro a acessibilidade e substituição de launcher do Projectivy para evitar concorrência.
- Conferir que o APK é o build do commit `32cec457d333d7d78a8a5fa739e032754e4270be` ou posterior.
- GitHub debug APKs podem ter assinaturas diferentes entre máquinas/execuções. Preferir assinatura estável para futuras atualizações. Antes de uma reinstalação que apague dados, verificar todos os usuários com `pm list users` e obter aprovação.

## Diagnóstico ADB sem alterações
```powershell
$tv = "192.168.70.6:45527"
.\adb.exe -s $tv shell dumpsys accessibility | Select-String "HomeRedirectService" -Context 2,2
.\adb.exe -s $tv logcat -c
# Pressione Home uma vez na TV
.\adb.exe -s $tv logcat -d -s StreamMaxHomeRedirect:D
```
Note que `logcat -c` apenas limpa o buffer de logs, não dados de apps.

## Interpretação
- `HOME key seen` seguido de `trigger=key`: teclado HOME foi entregue ao serviço. Confirmar visualmente se não piscou.
- Só `window=com.google.android.apps.tv.launcherx`, `trigger=window-fallback`: sistema não entregou HOME (ou filtragem não foi aplicada); a piscada pode ser inerente ao método.
- Sem logs: verificar se o APK novo foi instalado e se o serviço está vinculado.
- Em caso de tela inicial instável: desativar acessibilidade do StreamMax Home e usar Google TV ou Projectivy.
