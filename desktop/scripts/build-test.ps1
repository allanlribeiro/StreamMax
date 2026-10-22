# Executar em PowerShell no Windows: .\scripts\build-test.ps1 -LogoPath "C:\...\streammax-s.png"
# Testa o projeto EXISTENTE antes de gerar o instalador de avaliação.
param(
  [Parameter(Mandatory=$false)]
  [string]$LogoPath
)
$ErrorActionPreference = "Stop"
Set-Location (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Write-Host "StreamMax Windows — compilação de teste (não assinada)"
if ($LogoPath) {
  if (!(Test-Path -LiteralPath $LogoPath -PathType Leaf)) {
    throw "Arquivo da logo S não encontrado: $LogoPath"
  }
  $ext = [System.IO.Path]::GetExtension($LogoPath).ToLowerInvariant()
  if ($ext -notin @(".png",".jpg",".jpeg")) {
    throw "A logo deve ser PNG ou JPG."
  }
  New-Item -ItemType Directory -Force -Path "assets" | Out-Null
  $target = if ($ext -eq ".png") {"assets\streammax-s.png"} else {"assets\streammax-s.jpg"}
  Copy-Item -LiteralPath $LogoPath -Destination $target -Force
}
if (!(Test-Path "assets\streammax-s.png") -and !(Test-Path "assets\streammax-s.jpg")) {
  throw "Inclua a arte S aprovada em desktop\assets\streammax-s.png."
}
foreach ($tool in @("node","npm","python")) {
  if (!(Get-Command $tool -ErrorAction SilentlyContinue)) {
    throw "Ferramenta não encontrada no PATH: $tool"
  }
}
Write-Host "1/5 Instalando dependências"
npm install --no-audit --no-fund
if ($LASTEXITCODE -ne 0) { throw "Falha ao instalar dependências" }
Write-Host "2/5 Testando módulos existentes"
npm test
if ($LASTEXITCODE -ne 0) { throw "Falha nos testes; compilação interrompida" }
Write-Host "3/5 Validando janela Electron e comunicação interna"
$env:STREAMMAX_SMOKE_TEST = "1"
try {
  npm run smoke
  if ($LASTEXITCODE -ne 0) { throw "Falha no smoke test do Electron" }
} finally {
  Remove-Item Env:\STREAMMAX_SMOKE_TEST -ErrorAction SilentlyContinue
}
Write-Host "4/5 Gerando ícone oficial"
python scripts/make_icon.py
if ($LASTEXITCODE -ne 0) { throw "Falha na geração do ícone" }
Write-Host "5/5 Compilando instalador unsigned Windows"
npm run dist
if ($LASTEXITCODE -ne 0) { throw "Falha na geração do instalador" }
Write-Host "Compilação encerrada. Veja os arquivos em desktop\dist."
