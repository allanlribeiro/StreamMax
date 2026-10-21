# StreamMax Home Teste — TCL P7K

Launcher Android TV experimental, escrito em Java com Views nativas. Projeto independente dentro de uma pasta e branch isoladas do StreamMax. Referência conceitual: [dumbtv](https://github.com/humblemuzzu/dumbtv), sem executar seus scripts de Fire OS.

## Objetivo

- Tela inicial sem anúncios, sem recomendações, sem feed, sem vídeos de fundo.
- Sem permissão `INTERNET`, acessibilidade, root ou alteração dos serviços do sistema.
- Relógio, data, paisagem estática, atalhos e fileira com os aplicativos instalados.
- StreamMax priorizado no início da fileira; navegação por D-pad e seleção pelo controle.
- Abrir Configurações; manter HOME original durante a fase de teste.

## Segurança / primeira versão

**Este APK NÃO assume o botão HOME e NÃO substitui o Google TV.** Ele aparece como "StreamMax Home Teste" na lista de aplicativos. O atalho Voltar retorna à interface anterior.

**Não executar** `scripts/all.sh` ou `scripts/debloat.sh` do dumbtv na TCL. São alvos pensados especialmente para Fire TV e a própria documentação do projeto alerta sobre pacotes exclusivos de Google TV.

## Instalação experimental

1. Abrir o workflow **StreamMax Home Teste — Android TV** no GitHub Actions.
2. Aguardar o resultado verde e baixar o artefato `StreamMax-Home-Teste-<run>`.
3. Extrair o ZIP do artefato e instalar `StreamMax-Home-Teste.apk` na TV.
4. Abrir "StreamMax Home Teste" pela tela atual, verificar controle, aplicativos, configurações e velocidade. A versão instalada não modifica a home padrão.
5. Para desfazer, desinstalar apenas `com.streammax.home.teste`.

A CI gera um **APK de debug**, adequado apenas para testes; releases futuros exigem assinatura permanente antes de serem distribuídos como atualizações.

## Compilação local

JDK 17, Android SDK 35 e Gradle 8.10.2. Dentro de `streammax-home`: `gradle :app:assembleDebug`.

## Próximas etapas depois do teste real

- Favoritos editáveis e organização de aplicativos.
- Atalhos HDMI específicos para TCL, após inventário ADB somente leitura.
- Wallpaper próprio e refinamento visual, sem vídeo e sem GIF.
- Medir RAM/fluidez e comparar com Google TV.
- **Apenas após aprovação explícita:** plano de troca do launcher, com backup (apps, settings global/system/secure e memória), log de cada comando `pm disable-user` e script de recuperação com `pm enable`. Não mexer em TV/antena/HDMI, controle, Bluetooth, teclado, Google Play, WebView e atualizações.

O launcher não remove anúncios **dentro** de YouTube, outros aplicativos ou da tela Google TV original enquanto ela continuar em uso.
