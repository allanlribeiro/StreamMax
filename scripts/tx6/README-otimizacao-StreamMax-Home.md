# StreamMax Home RC2 — plano de desempenho para TV Box TX6 (2 GB RAM)

## Estado validado pelo usuario
- APK original RC2 (`com.streammax.home.rc2`) instalado.
- HOME padrao: `com.streammax.home.rc2/com.streammax.tvdetox.MainActivity`.
- Launcher original Oranth (`com.oranth.tvlauncher`) desativado reversivelmente usando `pm disable-user --user 0`, nao desinstalado.
- Inicio direto no StreamMax Home apos reboot, navegacao pelo controle e apps testados.
- Manter a aparencia original da RC2; **nao substituir pela UI simplificada da RC3/RC4**.
- Versao auxiliar TX6 nao e necessaria para o botao HOME quando o sistema respeita `set-home-activity`.

## Passo 1 — medir, nao presumir
Execute `scripts/tx6/Diagnostico-StreamMax-Home-TX6.ps1` no PowerShell dentro do diretorio platform-tools, mantendo o app aberto, TV Box ligada e ADB conectado.

O script verifica o device TX6, confirma RC2 habilitada, e coleta:
- Resolucao/densidade real; propriedades basicas do renderizador.
- Memoria disponivel `/proc/meminfo` e PSS da RC2 em 3 amostras.
- Resumo dos processos e `dumpsys gfxinfo` do pacote da RC2.
- Launcher HOME resolvido e lista dos pacotes desativados.
- Salva relatorios no PC em pasta `Diagnostico_TX6_Home_...`.

O script **nao altera nenhuma configuracao na TX6**. Solicitar apenas `RESUMO.txt` e indicadores pertinentes; nao enviar dados privados se nao desejar.

## Como priorizar correcoes

1. **PSS da RC2 alto ou crescente?** Investigar alocacoes de bitmaps, lista de apps, cache de icones, ciclo de vida. Comparar 3 amostras e um teste repetido apos uso. Nao declarar vazamento sem evidencia.
2. **Muitos janky frames?** Fazer testes reais de navegacao com gfxinfo; revisar trabalhos no thread principal e invalidacoes do layout. Reduzir animacoes do proprio launcher se estiverem causando frame drops.
3. **Abertura de outros apps lenta?** Medir com `am start -W` antes/depois, separando `ThisTime`/`TotalTime` e cache quente/frio. Evitar `force-stop` em apps sem motivo e sem autorizacao.
4. **RAM disponivel estavel e launcher leve?** Nao mudar por mudar. Escalas globais de animacao em 0.5x alteram sensacao visual, mas nao reduzem necessariamente tempo real de carregamento.

## Bloqueio para alterar o APK RC2
Nao existe fonte Java/Gradle original RC2 confirmada neste repositorio. A documentacao indica que o JPG original e o APK foram recuperados, mas as classes de layout **nao estao comprovadamente reconstruidas**. APK nao equivale ao codigo-fonte. Modificacoes do binario exigiriam recompilacao e chave de assinatura compativel; como alternativa, novo applicationId e instalacao lado a lado para teste. **Nao compilar UI substituta nem sobrepor a RC2 atual sem verificacao visual e de assinatura**.

## Restauracao se launcher deixar de funcionar
Pelo ADB enquanto conectado:
```powershell
.\adb.exe -s 192.168.0.106:5555 shell pm enable --user 0 com.oranth.tvlauncher
```
Nao desativar Google Play, WebView, Bluetooth, teclado, rede, HDMI ou componentes de OTA. Nao reiniciar a TX6 durante testes de uma nova APK antes de conferir rotas de recuperacao.
