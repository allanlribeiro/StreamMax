# Ler memoria apos reinicializacao para comparar com snapshot PREVIO.
# Somente leituras ADB. Nao altera a TV nem executa reinicio automaticamente.
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$BackupPasta,
    [Parameter(Mandatory=$true)][string]$Dispositivo
)
$ErrorActionPreference='Stop'
$BackupPasta=(Resolve-Path -LiteralPath $BackupPasta).Path
$original=Join-Path $BackupPasta 'memoria-proc.txt'
if (-not (Test-Path -LiteralPath $original)) { throw 'Backup de memoria original nao encontrado.' }
function Disponivel([string]$texto) {
    if ($texto -match '(?m)^MemAvailable:\s+(\d+)\s+kB') {
        return [math]::Round(([double]$Matches[1]/1024),1)
    }
    throw 'MemAvailable nao encontrado no /proc/meminfo.'
}
$antes=Disponivel (Get-Content -LiteralPath $original -Raw)
$depoisTexto=(& adb -s $Dispositivo shell cat /proc/meminfo | Out-String)
if ($LASTEXITCODE -ne 0) { throw 'ADB desconectado ou leitura de memoria falhou.' }
$depois=Disponivel $depoisTexto
$arquivo=Join-Path $BackupPasta 'memoria-apos-reinicio.txt'
Set-Content -LiteralPath $arquivo -Value $depoisTexto -Encoding UTF8
$delta=[math]::Round($depois-$antes,1)
$texto=@(
    "MemAvailable antes: $antes MB",
    "MemAvailable depois: $depois MB",
    "Diferenca: $delta MB",
    'ATENCAO: reiniciar a TV tambem libera memoria; isto NAO prova ganho por desativacao.',
    'Teste o controle, antena, HDMI, casting, Play Store e o StreamMax.'
)
$saida=Join-Path $BackupPasta 'comparacao-memoria.txt'
Set-Content -LiteralPath $saida -Value $texto -Encoding UTF8
Write-Host ($texto -join [Environment]::NewLine)
Write-Host "Relatorio salvo em: $saida"
