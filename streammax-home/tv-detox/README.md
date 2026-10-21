# Integracao segura com TV Detox — TCL P7K

Referência: [Ansub/tv-detox](https://github.com/Ansub/tv-detox) (Ansub Khan, licença MIT).

Este diretório é uma **adaptação independente do fluxo de diagnóstico** do TV Detox para o StreamMax Home. Não contém a interface original nem executa o instalador/limpeza do projeto de terceiros. **A primeira etapa só coleta informações; não modifica a TV.**

## Implementado nesta branch (21/10/2026)

- Código-fonte da home do [Ansub/tv-detox](https://github.com/Ansub/tv-detox), revisão `b937fa5ce43bff13279f014a0ad37b459f49319d` (MIT, licença preservada), adaptado como APK de **teste** com pacote próprio `com.streammax.tvdetox.teste`.
- Tela sem anúncios, fontes nativas, degradê azul metálico estático, sem `INTERNET`, sem launcher HOME e sem interface de desinstalação.
- Bloco **TV Aberta** fixo em primeira posição; YouTube, Netflix, Prime e demais apps instalados; Entradas e Configurações apenas no topo.
- O bloco TV Aberta só tenta um **canal real de `TYPE_TUNER`**, não a última entrada de TV. Se a TCL não expuser canais pela API padrão, ele informa que falta mapear a antena por ADB; **não muda para HDMI por engano**. A promessa "sempre na antena" depende do teste no dispositivo.
- `Coletar-Diagnostico.ps1`: backup integral do inventário e de settings global/system/secure, memória, HOME atual, candidatos HOME e entradas TV, e CSV de três grupos.
- `Aplicar-Mudancas-Aprovadas.ps1`: solicita confirmação separada para cada grupo e cada pacote elegível, aplica somente `pm disable-user --user 0` numa lista fechada sem TCL/fabricante, e pode zerar animações e desligar o protetor após aprovação. Cria log e scripts `desfazer-tudo.sh` e `desfazer-tudo.ps1` ANTES de modificar.
- `Comparar-Memoria.ps1`: leitura após a reinicialização para comparação de `MemAvailable`, sem afirmar que todo ganho decorre das alterações.
- Nenhum comando da TV é executado no GitHub: os scripts precisam ser rodados localmente, após autorização na TCL.

## Sequência recomendada na sua casa

1. Ativar depuração e autorizar o PC na TCL; instalar o Platform Tools (ADB) no Windows.
2. Rodar `Coletar-Diagnostico.ps1 -Dispositivo "IP:PORTA"`; guardar a pasta de backups **no PC**.
3. Abrir `home-candidatos.txt`, `tv-inputs.txt` e `classificacao-para-revisao.csv`. Validar com o usuário cada grupo e a entrada real de antena.
4. Se aprovado, executar `Aplicar-Mudancas-Aprovadas.ps1 -BackupPasta "PASTA" -Dispositivo "IP:PORTA" -ZerarAnimacoes -DesligarProtetor`. A confirmação interativa é obrigatória; não haverá desativações automáticas nem de apps TCL.
5. Após o backup, instalar **somente** o APK de teste compilado pelo workflow `StreamMax Home TV Detox — Teste TCL`. Ele não assume HOME. Testar foco, atalhos, entradas e bloco de antena.
6. Analisar os aplicativos HOME disponíveis e somente depois discutir mudança de home padrão por meio suportado, mantendo a Google TV original habilitada.
7. Reiniciar pelo menu da TV (não tirar da tomada), reconectar ADB e executar `Comparar-Memoria.ps1 -BackupPasta "PASTA" -Dispositivo "IP:PORTA"`. Testar novamente TV, HDMI, controle, casting, Play Store e StreamMax.
8. Para restaurar, no Git Bash: `sh desfazer-tudo.sh IP:PORTA`; ou no PowerShell: `.\desfazer-tudo.ps1 -Dispositivo "IP:PORTA"`. Se foi apenas instalada a home teste, use a tela de aplicativos da TV para removê-la quando quiser, sem tocar no launcher original. Depois desative a depuração.

Os scripts de backup são **somente locais**. Não envie publicamente as configurações completas (`settings-*.txt`, `pacotes-flags.txt` etc.).

## Objetivo

Unir duas frentes independentes:

1. **StreamMax Home:** launcher Java com a aparência aprovada — azul metálico, papel de parede estático, relógio, favoritos, brilho leve ao focar; atalhos de Wi-Fi, entradas e configurações apenas após confirmar os intents funcionais da P7K.
2. **Diagnóstico/otimização ADB:** identificar apps e serviços, medir memória, preservar um snapshot no PC, propor mudanças individualmente, medir antes/depois e permitir restaurá-las.

## Avisos específicos do Google TV / TCL

- **Nunca desligar** o Google TV Home `com.google.android.apps.tv.launcherx` e o Setup `com.google.android.tungsten.setupwraith`. O próprio TV Detox relata reinicializações problemáticas em algumas Google TVs.
- Preservar `com.tcl.tvinput`, `com.tcl.suspension`, `com.tcl.dashboard`, `com.tcl.partnercustomizer`, controle remoto, Bluetooth, teclado, Play Store, WebView e atualizações. Alguns atalhos do controle TCL passam pelo dashboard/customizer.
- Não executar diretamente `install.sh`, `undo.sh`, `check.sh --fix` nem rotinas de debloat de outras TVs. O `undo.sh` original só conhece alterações feitas pelo instalador dele.
- Uma classificação como `safe` no catálogo de outro televisor não é autorização para desligar pacotes na P7K.
- Sem root. Antes de qualquer alteração futura, exigir o aceite explícito do usuário e restringir desativações a `pm disable-user --user 0`, nunca a `pm uninstall`.
- A diferença de memória deve ser medida com condições semelhantes, inclusive após reinicializar, porque reiniciar por si só pode liberar RAM. Nenhum ganho de desempenho é garantido.
- Após os testes, desligar a depuração USB/rede nas configurações de desenvolvedor.

## Etapa 1 — preparar o computador

Windows 10/11, PowerShell e [Android SDK Platform Tools](https://developer.android.com/tools/releases/platform-tools). A TV e o computador devem estar na mesma rede, caso a conexão seja ADB por Wi-Fi.

Primeiro ativar e autorizar a depuração na TV. Dependendo da versão do Google TV, a conexão poderá ser por `adb pair IP:PORTA` seguido de `adb connect IP:PORTA` (**portas diferentes**) ou por `adb connect IP:5555`. **Não presumir que a P7K usa a porta 5555**.

Após aparecer como `device` em:

```powershell
adb devices
```

Executar a coleta (no PowerShell, dentro desta pasta):

```powershell
.\Coletar-Diagnostico.ps1 -Dispositivo "IP:PORTA"
```

Se a política de scripts local impedir a execução, abrir um novo processo PowerShell sem alterar a política permanente do computador:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Coletar-Diagnostico.ps1 -Dispositivo "IP:PORTA"
```

**No início, não rode comandos ADB de desativação.** O relatório é criado em `Documentos/StreamMaxHome-Backups/TCL-P7K-DATA-HORA`.

## Dados coletados

- Todos os pacotes, pacotes instalados pelo usuário e os já desativados;
- Settings `global`, `system`, `secure` **completos**;
- Memória `/proc/meminfo` (inclui `MemAvailable`), `dumpsys meminfo`;
- Modelo, versão do firmware, launcher responsável por HOME e teclado ativo;
- Flags dos pacotes, incluindo `PERSISTENT`;
- `classificacao-para-revisao.csv`: essenciais, aplicativos do usuário, possíveis candidatos e desconhecidos.

Os arquivos podem conter identificadores e configurações privadas: **não os enviar para GitHub, chat público ou outros serviços**. Para discutir a lista comigo, basta copiar **somente** os nomes dos pacotes e seus grupos do CSV, evitando configurações e dados pessoais.

## Etapa 2 — depois da análise com o usuário

O script interativo de aplicação já está preparado, mas **não está autorizado e não foi executado** na TV. Ele deve ser revisado e usado apenas após confirmar backup e compatibilidade:

1. Conferir manualmente os três grupos, inclusive os pacotes não classificados.
2. Pedir aprovação antes de cada grupo e de cada mudança, preservando tudo que afeta HDMI, remoto, TV, atualizações e os aplicativos usados.
3. Manter registro por alteração e respectivo `pm enable --user 0`; criar `desfazer-tudo` a partir desse registro.
4. Executar poucas alterações de cada vez, testar controle/HDMI/casting/StreamMax, reiniciar uma vez e medir novamente.
5. Somente depois dos testes do APK atual considerar configurar uma home padrão por método suportado, **sem** desativar `launcherx`.

O APK de teste atual **não** é responsável pelo botão HOME. Trocar o launcher é etapa separada.

## Referência e atribuição

O TV Detox é trabalho de Ansub Khan, disponível sob licença MIT:
[Repositório e licença](https://github.com/Ansub/tv-detox/blob/main/LICENSE).

Esta implementação é uma rotina própria de snapshot e triagem adaptada às restrições da TCL P7K e não inclui o código ou os scripts upstream.
