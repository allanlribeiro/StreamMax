# StreamMax Home — configurar assinatura permanente no Windows

A RC3 instalada está assinada com chave de depuração, então **a primeira migração** para a chave permanente não instalará por cima. Faça isso só depois de salvar a chave e validar o APK de release.

## 1. Criar a chave privada LOCALMENTE (não enviar ao chat nem ao GitHub)
Abra PowerShell em seu computador Windows com `keytool` (JDK) disponível:

```powershell
$dir = Join-Path $HOME "Documents\StreamMax-Home-Signing"
New-Item -ItemType Directory -Force -Path $dir | Out-Null
keytool -genkeypair -v -keystore (Join-Path $dir "streammax-home-release.p12") -storetype PKCS12 -alias streammax-home -keyalg RSA -keysize 3072 -validity 10000 -dname "CN=StreamMax Home, OU=Development, O=Personal, C=BR"
```

Escolha uma senha forte no prompt. Não divulgue a senha ou o arquivo `.p12`. Faça backup seguro e separado da chave e da senha. Em PKCS12, normalmente a senha da chave é igual à senha do armazenamento.

## 2. Configurar GitHub Actions Secrets
Em `allanlribeiro/StreamMax`, abra Settings > Secrets and variables > Actions e crie:

| Secret | Conteúdo |
|---|---|
| `STREAMMAX_HOME_KEYSTORE_B64` | Base64 do conteúdo **binário** do `.p12` |
| `STREAMMAX_HOME_STORE_PASSWORD` | Senha do armazenamento |
| `STREAMMAX_HOME_KEY_ALIAS` | `streammax-home` |
| `STREAMMAX_HOME_KEY_PASSWORD` | Senha da chave (geralmente igual à do armazenamento) |

Para gerar Base64 local no PowerShell sem imprimir no terminal:

```powershell
$path = Join-Path $HOME "Documents\StreamMax-Home-Signing\streammax-home-release.p12"
[Convert]::ToBase64String([IO.File]::ReadAllBytes($path)) | Set-Clipboard
```

Cole somente no campo do **secret dentro do GitHub**, e depois limpe a área de transferência:

```powershell
Set-Clipboard -Value ""
```

Atenção: clipboard pode estar sincronizado entre dispositivos. Se isso for uma preocupação, prefira a CLI `gh secret set` com entrada padrão em vez do clipboard. Jamais cole a senha nem a chave em issue, log, código ou conversa.

## 3. Compilar
Workflow: `.github/workflows/streammax-home-stable-release.yml`. Ele exige os secrets; sem eles falha intencionalmente.

Como o workflow vive em uma branch de desenvolvimento, a opção manual poderá depender da configuração do GitHub Actions/default branch. Não mescle no branch principal sem revisão.

## 4. Migração controlada da TCL
Somente após build com sucesso, conferir a assinatura do APK e ter backup da chave. A troca de assinatura da RC3 debug para release exige desinstalar a RC3 dos usuários Android onde estiver instalada, após desabilitar seu serviço de acessibilidade e consentimento. Preservar RC2, Projectivy e Google TV.

## 5. Futuras atualizações
Manter **o mesmo keystore e alias, applicationId `com.streammax.home.rc3`, e incrementar versionCode**. Só assim `adb install -r` poderá funcionar sem perda de dados.
