# StreamMax Home TCL P7K - aplicacao conservadora e inteiramente reversivel.
# Requer um backup produzido antes por Coletar-Diagnostico.ps1.
# NUNCA desinstala apps; NUNCA desativa home, TCL, HDMI, controle ou sistema.
# Requer confirmacao interativa por grupo, pacote e configuracao.
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$BackupPasta,
    [Parameter(Mandatory=$true)][string]$Dispositivo,
    [switch]$ZerarAnimacoes,
    [switch]$DesligarProtetor
)
$ErrorActionPreference = 'Stop'
if ($Dispositivo -notmatch '^[A-Za-z0-9.:_\-]+$') { throw 'Dispositivo ADB invalido.' }
if (-not (Get-Command adb -ErrorAction SilentlyContinue)) { throw 'adb.exe nao encontrado.' }
$BackupPasta = (Resolve-Path -LiteralPath $BackupPasta).Path
$necessarios = @(
    'settings-global.txt','settings-system.txt','settings-secure.txt',
    'pacotes-instalados.txt','pacotes-desabilitados.txt','memoria-proc.txt',
    'home-atual.txt','classificacao-para-revisao.csv','valores-para-restaurar.csv'
)
foreach ($nome in $necessarios) {
    if (-not (Test-Path -LiteralPath (Join-Path $BackupPasta $nome))) {
        throw "Backup incompleto: $nome. Refaça a coleta ANTES de mudar algo."
    }
}
$estado = (& adb -s $Dispositivo get-state | Out-String).Trim()
if ($LASTEXITCODE -ne 0 -or $estado -ne 'device') { throw 'Conexao ADB indisponivel.' }
$bas = Get-Content -LiteralPath (Join-Path $BackupPasta 'pacotes-instalados.txt')
$instalados = @{}
foreach ($linha in $bas) { if ($linha -match '^package:(\S+)$') { $instalados[$Matches[1]]=$true } }
$origOff = @{}
foreach ($linha in (Get-Content -LiteralPath (Join-Path $BackupPasta 'pacotes-desabilitados.txt'))) {
    if ($linha -match '^package:(\S+)$') { $origOff[$Matches[1]]=$true }
}
$grupos = @(Import-Csv -LiteralPath (Join-Path $BackupPasta 'classificacao-para-revisao.csv'))
if ($grupos.Count -ne $instalados.Count -or $grupos.Count -eq 0) {
    throw 'Inventario de apps divergente/incompleto. Refaca a coleta.'
}
$atual = @(& adb -s $Dispositivo shell pm list packages)
if ($LASTEXITCODE -ne 0) { throw 'Nao foi possivel conferir os apps da TV.' }
$hoje = @{}
foreach ($linha in $atual) { if ($linha -match '^package:(\S+)$') { $hoje[$Matches[1]]=$true } }
if ($hoje.Count -ne $instalados.Count) {
    throw 'Lista de pacotes mudou desde o backup. Refaca a coleta antes de prosseguir.'
}
foreach ($p in $instalados.Keys) {
    if (-not $hoje.ContainsKey($p)) { throw "Pacote ausente desde o backup: $p" }
}

function Confirmar([string]$pergunta) {
    $resposta = Read-Host "$pergunta [digite SIM]"
    return $resposta -ceq 'SIM'
}
function Ler-Estado([string]$ns, [string]$nome) {
    $valor = (& adb -s $Dispositivo shell settings get $ns $nome | Out-String).Trim()
    if ($LASTEXITCODE -ne 0) { throw "Erro ao consultar $ns/$nome" }
    return $valor
}
# Lista fechada: mesmo que o CSV diga "candidato", nenhum pacote fora daqui
# pode ser desativado pelo script. Nao inclui NENHUM com.tcl.*.
$listaRestrita = @(
    'com.google.android.tvrecommendations',
    'com.google.android.backdrop',
    'com.google.android.feedback'
)
$permitidos = @{}
foreach ($p in $listaRestrita) { $permitidos[$p]=$true }

$candidatos = @($grupos | Where-Object { $_.Grupo -like 'Candidatos:*' })
$tv = @($grupos | Where-Object { $_.Grupo -like 'TV precisa*' })
$usados = @($grupos | Where-Object { $_.Grupo -like 'Apps que voce usa*' })
Write-Host ''
Write-Host ('GRUPO 1 - Candidatos para revisao (' + $candidatos.Count + ')')
$candidatos | Format-Table Pacote, Estado, Persistente -AutoSize
Write-Host ('GRUPO 2 - TV precisa / desconhecidos (' + $tv.Count + '): SEM MODIFICACOES')
$tv | Format-Table Pacote, Estado -AutoSize
Write-Host ('GRUPO 3 - Apps que voce usa (' + $usados.Count + '): SEM MODIFICACOES')
$usados | Format-Table Pacote, Estado -AutoSize
Write-Host ''
Write-Host 'ATENCAO: na TCL Google TV, desativar apps pode causar perda de funcoes.'
Write-Host 'O launcher Google TV, os aplicativos TCL e servicos protegidos SEMPRE ficam ligados.'
if (-not (Confirmar 'Aprova preservar integralmente o grupo 2 (TV e desconhecidos)?')) { return }
if (-not (Confirmar 'Aprova preservar integralmente o grupo 3 (apps do usuario)?')) { return }
if (-not (Confirmar 'Aprova apenas AVALIAR o grupo 1, sem desativacao automatica?')) { return }

$escolhidos = @()
foreach ($row in $candidatos) {
    $p = $row.Pacote
    if (-not $permitidos.ContainsKey($p) -or $origOff.ContainsKey($p) -or
        $row.Persistente -eq 'True' -or $p -match '^com\.tcl\.' -or
        $p -match '(?i)(launcher|home|system|setup|ota|update|tvinput|remote|webview|bluetooth|keyboard)') {
        Write-Host "PRESERVADO (sem permissao para mudar): $p"
        continue
    }
    if (Confirmar "Desativar SOMENTE $p?") { $escolhidos += $p }
}
$chaves = @()
if ($ZerarAnimacoes) {
    if (Confirmar 'Zerar as 3 escalas de animacoes e conservar valores antigos?') {
        $chaves += @(
            [pscustomobject]@{ Ns='global'; Nome='window_animation_scale'; Novo='0' },
            [pscustomobject]@{ Ns='global'; Nome='transition_animation_scale'; Novo='0' },
            [pscustomobject]@{ Ns='global'; Nome='animator_duration_scale'; Novo='0' }
        )
    }
}
if ($DesligarProtetor) {
    Write-Host 'Aviso: protetor de tela desligado nao e adequado a longos periodos de imagem estatica, especialmente em OLED.'
    if (Confirmar 'Desligar o protetor de tela, guardando os valores originais?') {
        $chaves += @(
            [pscustomobject]@{ Ns='secure'; Nome='screensaver_enabled'; Novo='0' },
            [pscustomobject]@{ Ns='secure'; Nome='screensaver_activate_on_sleep'; Novo='0' },
            [pscustomobject]@{ Ns='secure'; Nome='screensaver_activate_on_dock'; Novo='0' }
        )
    }
}
if ($escolhidos.Count -eq 0 -and $chaves.Count -eq 0) {
    Write-Host 'Nenhuma alteracao aprovada. Nada foi executado.'; return
}

# Conferir valores do backup antes de qualquer aplicacao.
$originais = @{}
foreach ($r in (Import-Csv -LiteralPath (Join-Path $BackupPasta 'valores-para-restaurar.csv'))) {
    $originais[($r.Namespace + '/' + $r.Chave)] = $r.Valor
}
foreach ($key in $chaves) {
    $id = $key.Ns + '/' + $key.Nome
    if (-not $originais.ContainsKey($id)) { throw "Valor original ausente: $id" }
    $valorAtual = Ler-Estado $key.Ns $key.Nome
    if ($valorAtual -cne $originais[$id]) {
        throw "A configuracao $id mudou depois do backup. Refaca a coleta."
    }
    if ($originais[$id] -notmatch '^(null|[0-9]+(\.[0-9]+)?)$') {
        throw "Valor de configuracao fora da lista segura: $id. Nada alterado."
    }
}
$undoSh = Join-Path $BackupPasta 'desfazer-tudo.sh'
$undoPs = Join-Path $BackupPasta 'desfazer-tudo.ps1'
$log = Join-Path $BackupPasta 'alteracoes-executadas.log'
if ((Test-Path -LiteralPath $log) -or (Test-Path -LiteralPath $undoSh)) {
    throw 'Esse backup ja possui plano/log de alteracoes. Nao sobrescrever. Revisar ou fazer novo backup.'
}
$header = @(
    '#!/bin/sh',
    '# StreamMax Home TCL P7K: desfazer APENAS as alteracoes aprovadas.',
    '# Uso: sh desfazer-tudo.sh IP:PORTA',
    'set -eu',
    'TV="$1"',
    'if [ -z "$TV" ]; then echo "Informe IP:PORTA do dispositivo ADB." >&2; exit 1; fi',
    'adb -s "$TV" get-state >/dev/null',
    'echo "Restaurando alteracoes do StreamMax Home..."'
)
# Arquivos criados ANTES da primeira mudanca. ASCII evita BOM em /bin/sh.
Set-Content -LiteralPath $undoSh -Value $header -Encoding ASCII
$psHeader = @(
    'param([Parameter(Mandatory=$true)][string]$Dispositivo)',
    '# Restauracao local alternativa para PowerShell',
    '$ErrorActionPreference = "Stop"',
    '& adb -s $Dispositivo get-state | Out-Null',
    'if ($LASTEXITCODE -ne 0) { throw "ADB desconectado." }'
)
Set-Content -LiteralPath $undoPs -Value $psHeader -Encoding UTF8
Set-Content -LiteralPath $log -Encoding UTF8 -Value @(
    "# StreamMax Home alteracoes APROVADAS - $(Get-Date -Format o)",
    "# Backup: $BackupPasta",
    '# As linhas PLANEJADO possuem o comando inverso ANTES da alteracao.'
)
function Registra([string]$tipo, [string]$cmd, [string]$undo) {
    Add-Content -LiteralPath $log -Value ("$(Get-Date -Format o) | $tipo | $cmd | DESFAZER: $undo") -Encoding UTF8
}

foreach ($p in $escolhidos) {
    # Recusar qualquer alteracao em TCL/sistema mesmo diante de manipulacao do CSV.
    if (-not $permitidos.ContainsKey($p) -or $p -match '^com\.tcl\.' -or
        -not $instalados.ContainsKey($p) -or $origOff.ContainsKey($p)) {
        throw "Bloqueado por politica de seguranca: $p"
    }
    $reverse = "pm enable --user 0 $p"
    Add-Content -LiteralPath $undoSh -Encoding ASCII -Value ('adb -s "$TV" shell ' + $reverse)
    Add-Content -LiteralPath $undoPs -Encoding UTF8 -Value ('& adb -s $Dispositivo shell ' + $reverse)
    Registra 'PLANEJADO' "pm disable-user --user 0 $p" $reverse
    & adb -s $Dispositivo shell pm disable-user --user 0 $p | Out-Null
    if ($LASTEXITCODE -ne 0) { Registra 'FALHOU' $p $reverse; throw "Falha ao desativar $p" }
    $desabilitados = @(& adb -s $Dispositivo shell pm list packages -d)
    if ($LASTEXITCODE -ne 0 -or -not ($desabilitados -contains "package:$p")) {
        Registra 'NAO_CONFIRMADO' $p $reverse
        throw "Estado de $p nao confirmado; execute desfazer-tudo antes de prosseguir."
    }
    Registra 'CONFIRMADO' $p $reverse
}
foreach ($key in $chaves) {
    $id = $key.Ns + '/' + $key.Nome
    $old = [string]$originais[$id]
    $cmd = "settings put $($key.Ns) $($key.Nome) $($key.Novo)"
    if ($old -eq 'null') { $reverse = "settings delete $($key.Ns) $($key.Nome)" }
    else { $reverse = "settings put $($key.Ns) $($key.Nome) $old" }
    Add-Content -LiteralPath $undoSh -Encoding ASCII -Value ('adb -s "$TV" shell ' + $reverse)
    Add-Content -LiteralPath $undoPs -Encoding UTF8 -Value ('& adb -s $Dispositivo shell ' + $reverse)
    Registra 'PLANEJADO' $cmd $reverse
    & adb -s $Dispositivo shell settings put $key.Ns $key.Nome $key.Novo | Out-Null
    if ($LASTEXITCODE -ne 0 -or (Ler-Estado $key.Ns $key.Nome) -ne $key.Novo) {
        Registra 'FALHOU' $cmd $reverse
        throw "Falha ao ajustar $id. Execute desfazer-tudo."
    }
    Registra 'CONFIRMADO' $cmd $reverse
}
Add-Content -LiteralPath $undoSh -Encoding ASCII -Value 'echo "Restauracao solicitada. Reinicie e confira Home, HDMI, remoto e memoria."'
Add-Content -LiteralPath $undoPs -Encoding UTF8 -Value 'Write-Host "Restauracao solicitada. Reinicie e valide a TV."'
Write-Host ''
Write-Host "Mudancas concluidas. Log: $log"
Write-Host "Reversao (Git Bash): sh '$undoSh' $Dispositivo"
Write-Host "Reversao (Windows): powershell -File '$undoPs' -Dispositivo $Dispositivo"
Write-Host 'Ainda falta reiniciar pelo menu da TV e comparar a memoria.'
