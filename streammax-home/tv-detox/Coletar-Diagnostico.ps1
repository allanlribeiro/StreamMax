# StreamMax Home / TCL P7K - coleta ADB somente leitura
# Inspirado no inventario seguro do projeto TV Detox (Ansub Khan, MIT):
# https://github.com/Ansub/tv-detox
# NUNCA desinstala, desabilita, altera configuracoes nem muda o launcher.
# Requer Windows PowerShell 5.1+ e Android SDK Platform Tools (adb.exe).
[CmdletBinding()]
param(
    [string]$Dispositivo,
    [string]$Destino = "$env:USERPROFILE\Documents\StreamMaxHome-Backups"
)

$ErrorActionPreference = 'Stop'
if (-not (Get-Command adb -ErrorAction SilentlyContinue)) {
    throw 'adb.exe nao encontrado. Instale o Android SDK Platform Tools e configure o PATH.'
}

$linhas = @(& adb devices)
$disponiveis = @($linhas | ForEach-Object {
    if ($_ -match '^([^\s]+)\s+device\s*$') { $Matches[1] }
})
if (-not $Dispositivo) {
    if ($disponiveis.Count -ne 1) {
        throw 'Informe -Dispositivo IP:PORTA ou serial ADB. Conecte e autorize a TV primeiro.'
    }
    $Dispositivo = $disponiveis[0]
}
if ($Dispositivo -notmatch '^[A-Za-z0-9.:_\-]+$') {
    throw 'Serial do dispositivo invalido.'
}
$estado = (& adb -s $Dispositivo get-state 2>&1 | Out-String).Trim()
if ($LASTEXITCODE -ne 0 -or $estado -ne 'device') {
    throw "TV nao autorizada/conectada ($Dispositivo). Verifique a depuracao e o aceite na tela."
}

$agora = Get-Date -Format 'yyyyMMdd-HHmmss'
$pasta = Join-Path $Destino ("TCL-P7K-" + $agora)
New-Item -ItemType Directory -Path $pasta -Force | Out-Null
Write-Host "Backup local: $pasta"

function Guardar-Leitura {
    param([string]$Nome, [string[]]$Argumentos, [bool]$Obrigatorio = $true)
    $conteudo = @(& adb -s $Dispositivo @Argumentos 2>&1)
    $codigo = $LASTEXITCODE
    $texto = ($conteudo | Out-String).TrimEnd()
    Set-Content -LiteralPath (Join-Path $pasta $Nome) -Value $texto -Encoding UTF8
    if ($codigo -ne 0 -and $Obrigatorio) {
        throw "Falhou a leitura $Nome. Nenhuma alteracao foi feita na TV."
    }
    return $texto
}

# Toda configuracao solicitada: snapshots COMPLETOS e locais.
# Nao compartilhar esses arquivos publicamente; podem conter identificadores.
foreach ($namespace in @('global', 'system', 'secure')) {
    Guardar-Leitura -Nome ("settings-" + $namespace + ".txt") -Argumentos @('shell','settings','list',$namespace) | Out-Null
}
$instalados = Guardar-Leitura 'pacotes-instalados.txt' @('shell','pm','list','packages')
Guardar-Leitura 'pacotes-com-caminhos.txt' @('shell','pm','list','packages','-f') | Out-Null
$usuario = Guardar-Leitura 'pacotes-usuario.txt' @('shell','pm','list','packages','-3')
$desligados = Guardar-Leitura 'pacotes-desabilitados.txt' @('shell','pm','list','packages','-d')
Guardar-Leitura 'informacoes-modelo.txt' @('shell','getprop') | Out-Null
$mem = Guardar-Leitura 'memoria-proc.txt' @('shell','cat','/proc/meminfo')
Guardar-Leitura 'memoria-processos.txt' @('shell','dumpsys','meminfo') | Out-Null
Guardar-Leitura 'home-atual.txt' @('shell','cmd','package','resolve-activity','--brief','-a','android.intent.action.MAIN','-c','android.intent.category.HOME') | Out-Null
Guardar-Leitura 'teclado-atual.txt' @('shell','settings','get','secure','default_input_method') | Out-Null
$dump = Guardar-Leitura 'pacotes-flags.txt' @('shell','dumpsys','package')

# Lista exata de pacotes PERSISTENT extraida do dumpsys.
$persistent = @{}
$atual = ''
foreach ($linha in ($dump -split "\r?\n")) {
    if ($linha -match '^\s*Package \[([^\]]+)\]') { $atual = $Matches[1] }
    if ($atual -and $linha -match 'pkgFlags=\[[^\]]*\bPERSISTENT\b') {
        $persistent[$atual] = $true
    }
}
$doUsuario = @{}
foreach ($linha in ($usuario -split "\r?\n")) {
    if ($linha -match '^package:(.+)$') { $doUsuario[$Matches[1]] = $true }
}
$jaOff = @{}
foreach ($linha in ($desligados -split "\r?\n")) {
    if ($linha -match '^package:(.+)$') { $jaOff[$Matches[1]] = $true }
}

# Lista conservadora de sistema: nao desligar. Em caso de duvida, preservar.
$essenciais = @(
    'com.google.android.gms','com.google.android.gsf','com.android.vending',
    'com.google.android.apps.tv.launcherx','com.google.android.tungsten.setupwraith',
    'com.google.android.leanbacklauncher','com.google.android.tvlauncher',
    'com.android.systemui','com.android.settings','com.android.tv.settings',
    'com.android.bluetooth','com.google.android.inputmethod.latin',
    'com.android.inputmethod.latin','com.google.android.leanback.ime',
    'com.tcl.inputmethod.international','com.tcl.tvinput','com.tcl.suspension',
    'com.tcl.tv','com.tcl.providers.config','com.tcl.useragreement',
    'com.tcl.settings','com.tcl.dashboard','com.tcl.partnercustomizer',
    'com.tcl.tcl_bt_rcu_service','com.google.android.tv.remote.service',
    'com.google.android.apps.mediashell','com.google.android.webview',
    'com.tcl.android.webview','com.android.packageinstaller',
    'com.google.android.packageinstaller','com.dolby.android.audio.service',
    'com.streammax.home.teste','com.streammax.tv'
)
$essencial = @{}
foreach ($p in $essenciais) { $essencial[$p] = $true }

# Apenas CANDIDATOS; nao sao automaticamente seguros na sua P7K.
$candidatos = @(
    'com.google.android.tvrecommendations','com.google.android.backdrop',
    'com.tcl.waterfall.overseas','com.tcl.messagebox',
    'com.tcl.esticker','com.tcl.exhibit','com.tcl.ttvs','com.tcl.interactive'
)
$candidato = @{}
foreach ($p in $candidatos) { $candidato[$p] = $true }

$linhasPacotes = @($instalados -split "\r?\n" | Where-Object { $_ -match '^package:' })
$resultado = foreach ($linha in $linhasPacotes) {
    $p = $linha.Substring(8).Trim()
    $classe = 'Revisar / preservar ate identificar'
    if ($essencial.ContainsKey($p) -or $persistent.ContainsKey($p) -or
        $p -match '^com\.android\.providers\.' -or
        $p -match '(?i)(webview|keyboard|inputmethod|bluetooth|tvinput|firmware|updater|ota|update|setup|provision|systemui|remote|tuner|hdmi|cast)') {
        $classe = 'Essencial - nao desligar'
    } elseif ($doUsuario.ContainsKey($p)) {
        $classe = 'Aplicativo instalado pelo usuario - preservar'
    } elseif ($candidato.ContainsKey($p)) {
        $classe = 'Publicidade/telemetria - revisar antes de aprovar'
    }
    [pscustomobject]@{
        Grupo = $classe
        Pacote = $p
        Estado = $(if ($jaOff.ContainsKey($p)) { 'ja desligado' } else { 'habilitado' })
        Persistente = $persistent.ContainsKey($p)
    }
}
$resultado | Sort-Object Grupo, Pacote | Export-Csv -LiteralPath (Join-Path $pasta 'classificacao-para-revisao.csv') -NoTypeInformation -Encoding UTF8

$memDisponivel = ''
if ($mem -match '(?m)^MemAvailable:\s+(\d+)\s+kB') {
    $memDisponivel = [math]::Round([double]$Matches[1] / 1024, 1)
}
$resumo = @(
    'StreamMax Home - Diagnostico ANTES de qualquer mudanca'
    "Dispositivo: $Dispositivo"
    "Data: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
    "Pacotes detectados: $($resultado.Count)"
    "MemAvailable (estimativa, MB): $memDisponivel"
    'Nenhum app foi desativado, desinstalado ou configurado.'
    'Grupo "Publicidade/telemetria" e apenas sugestao para revisao manual.'
    'Nao compartilhar settings-*.txt ou pacotes-flags.txt publicamente.'
    'Passo seguinte: revisar classificacao-para-revisao.csv e pedir aprovacao.'
)
Set-Content -LiteralPath (Join-Path $pasta 'LEIA-ME.txt') -Value $resumo -Encoding UTF8
Write-Host ''
Write-Host ($resumo -join [Environment]::NewLine)
Write-Host ''
Write-Host 'Arquivos salvos apenas no computador; nenhum comando de alteracao foi executado.'
