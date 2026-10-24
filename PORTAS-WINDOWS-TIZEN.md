# StreamMax para Windows e Samsung Tizen

A interface usa o mesmo código C/SDL do StreamMax Android 1.0.17
(versão interna 2.0.24). Não é um APK executado dentro de um emulador.

## Windows

Alvo principal: Windows 11 de 64 bits; compatível também com Windows 10. Abra `StreamMax-1.0.17-Windows-x64.exe`.
É portátil; os dados ficam no perfil do usuário, separados do Android.
A primeira distribuição de testes não possui certificado comercial de assinatura.

- Mouse: clicar, rolar, selecionar texto e posicionar o cursor nos campos.
- Teclado: setas e Enter navegam; Escape volta; F11 alterna tela cheia.
- Campos: Backspace, Delete, Ctrl+A, seleção com Shift e área de transferência.
- O filme abre em uma janela do mpv incluída no pacote. Use a barra inferior
  com mouse, Espaço para pausar, setas para buscar e F para tela cheia.
  Q ou Escape retorna ao StreamMax, preservando o ponto de reprodução.
- Trailers continuam na interface. O mpv recebe as fontes por IPC local,
  incluindo os cabeçalhos exigidos pelos addons; endereços privados não vão
  para a linha de comando do processo.

A origem local é mantida entre execuções para conservar o IndexedDB.
O aplicativo mantém o isolamento do Electron; não usa Node dentro da página
nem desativa a segurança do Chromium.

## Samsung Tizen

O pacote é `StreamMax-1.0.17-tizen.wgt`, para Tizen 5.5 ou superior (modelos
2020+). O identificador é `StreamMax1.StreamMax`; não substitui o Nuvio.
O player usa o AVPlay da Samsung. Codecs, resolução e HDR dependem do modelo.

O WGT é distribuído sem assinatura. Para instalar em uma TV física:

1. Instale Tizen Studio com a extensão Samsung TV e o Certificate Manager.
2. Ative o modo de desenvolvedor em Apps da TV e informe o IP do computador.
3. Crie um perfil de certificado Samsung com o DUID da sua TV.
4. Extraia o WGT, assine a pasta com esse perfil e instale pela ferramenta.

```sh
unzip StreamMax-1.0.17-tizen.wgt -d streammax-widget
tizen package -t wgt -s SEU_PERFIL -- streammax-widget
sdb connect IP_DA_TV
tizen install -n streammax-widget/StreamMax.wgt -s SERIAL_DA_TV
```

O nome exato do arquivo assinado é informado pelo comando `tizen package`.
O certificado de distribuição contém os DUIDs autorizados: a assinatura de
um emulador não substitui a assinatura necessária para sua TV.

## Compilar e verificar

Use Emscripten 6.0.9. Configure `legacy-source/local.properties` a partir das
instruções existentes do projeto; não inclua contas, chaves pessoais de TMDB
ou Debrid nos pacotes. Os roteiros filtram a arte e removem dados pessoais.

```sh
cd legacy-source
bash tools/build-ass-wasm.sh
bash tools/build-webp-wasm.sh
EMSDK_DIR=/caminho/emsdk bash tools/tizen.sh
bash tools/tizen-wgt.sh
EMSDK_DIR=/caminho/emsdk bash tools/tizen.sh --desktop
```

Copie os arquivos finais de `build/windows-web/` para `desktop/web/`.
Instale o mpv x64 identificado em `desktop/THIRD-PARTY-NOTICES.txt`, mantendo
as licenças e conferindo o SHA-256 usado no workflow. Copie também
`desktop/node_modules/electron/dist/vulkan-1.dll` para `desktop/mpv/` antes
do empacotamento; o player necessita desse carregador mesmo com Direct3D.
No Windows:

```sh
npm ci --prefix desktop
npm --prefix desktop run test
npm --prefix desktop run dist:win
```

Os testes em `tests/ports/` verificam formulário SDL real, texto em português,
exclusão, seleção com mouse e senha oculta. O teste Windows também reproduz
uma mídia de teste gerada por FFmpeg, verifica cabeçalhos, pausa e busca.
O roteiro Samsung está preparado para instalar um WGT com certificado Samsung
no emulador oficial Tizen 10.0 e executar as verificações via depurador.
A execução do aplicativo nesse emulador ainda não foi validada: o emulador
iniciou e respondeu ao SDB, mas recusou o certificado genérico do SDK.
Desde TV Extension 7.0.1, a Samsung exige certificado Samsung também no emulador.
[Política oficial](https://developer.samsung.com/smarttv/develop/getting-started/using-sdk/tv-emulator/application-install-policy.html).

Para retomar, assine este WGT no seu computador com um perfil Samsung e
forneça somente o aplicativo assinado. Nunca envie senha, arquivo P12/PFX ou
chave privada. O workflow confere se o conteúdo do pacote corresponde ao
WGT original antes do teste. A chave privada permanece no seu computador.
O teste Chromium é uma bancada adicional, separada do teste no emulador.

As versões de teste dos portes usam uma pré-release separada. A versão
estável e o canal de atualização Android permanecem em 1.0.17. Nestas primeiras
versões Windows/Tizen, novas distribuições são instaladas manualmente.
