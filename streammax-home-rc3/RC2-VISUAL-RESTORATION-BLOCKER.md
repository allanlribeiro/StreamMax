# Bloqueio de layout: restaurar RC2 original antes de distribuir

**Pedido confirmado:** usuário deseja a interface original da RC2, não a RC3 simplificada nem o redesenho Metal experimental. Não alterar layout sem autorização explícita.

## Fonte de verdade
APK RC2 entregue na conversa: `StreamMax-Home-1.0.0-RC2.apk`.
Arte `res/drawable-nodpi-v4/streammax_metal_waves.jpg` recuperada do APK e incluída no projeto local `StreamMax-Home-RC2-visual-restaurado-fonte.zip`. **O JPG ainda não foi enviado para esta branch do GitHub. Não distribuir builds que simulem a RC2 sem ele.**

## Próxima integração
- Copiar a versão restaurada de `MainActivity.java` e o JPG original para `streammax-home-rc3/standalone/app/src/main`.
- Não usar o layout de duas linhas inventado na branch; reconstrução visual exata ainda não está comprovada.
- Preservar o HomeRedirectService funcional; assinatura protegida e applicationId `com.streammax.home.rc3`.
- Versão posterior deve ter `versionCode > 4`, usando a mesma chave permanente.
- Comparar na TCL visualmente com a RC2 original antes de qualquer substituição.

## Estado
ZIP local verificado quanto à integridade; APK assinado **não compilado nem instalado**. A versão atual na TV permanece intacta.
