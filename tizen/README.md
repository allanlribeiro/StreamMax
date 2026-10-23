# StreamMax para Samsung Tizen - prévia técnica 0.1.0

**Estado: experimental; ainda não testado em uma TV Samsung.**

## Origem e limites

A versão Android aprovada foi a 1.0.11, no branch release/streammax-1.0.11-ota-static-s-approved (commit 5a4a9cbcfa4a2186aefacb4955732b6a272b9d3f). O projeto Android usa Kotlin, Compose e Media3; seu APK NÃO roda em Samsung Tizen.

Para viabilizar uma primeira prova real no Tizen, este pacote reaproveita o motor Tizen GPLv3 do [Nuvio Native Legacy 2.0.8](https://github.com/iqui27/nuvio-native-legacy/tree/v2.0.8), mantendo seu código e suas atribuições. A alteração **apenas identifica o pacote, o ícone e o título de abertura como StreamMax Tizen Preview**.

**ATENÇÃO:** ainda NÃO é uma adaptação completa e idêntica do StreamMax 1.0.11 Android. A interface interna e os comportamentos de login, addons, legendas, reprodução, desempenho e atualização podem diferir.

## Compatibilidade

- Pacote StreamMax-Tizen-Preview-0.1.0-unsigned.wgt: Tizen 5.5 ou superior.
- Tizen 4.0/5.0 requer outra variante nativa, não coberta por este pacote.
- O WGT é intencionalmente **SEM ASSINATURA** e não pode ser instalado até ser assinado para a TV do usuário.
- Não existe atualização automática via StreamMax Android e a prévia não consta da loja Samsung.

## Onde obter o teste

Abra a aba Actions do repositório, workflow **StreamMax Tizen Preview (WGT sem assinatura)**, escolha um run bem-sucedido no branch feature/streammax-tizen-preview-20261023 e baixe o artifact **streammax-tizen-preview-unsigned**. Ele contém o WGT e um arquivo de checksum SHA-256.

## Instalação em uma Samsung (Windows)

1. Coloque TV e computador na **mesma rede**. Na TV, entre em Apps > App Settings e digite **12345** no controle. Ative **Developer Mode**, cadastre o IP local do seu computador e reinicie a TV.
2. Instale o Tizen Studio, **Samsung TV Extension** e **Samsung Certificate Extension**.
3. No **Certificate Manager**, crie o perfil **Samsung TV**, incluindo o **DUID da sua televisão** no certificado de distribuidor. Guarde o certificado e as senhas em local seguro. **Nunca envie as chaves ao GitHub.**
4. Baixe o ZIP do artifact, descompacte-o, e descompacte o WGT **unsigned** em uma nova pasta (os arquivos internos incluem config.xml, index.html, index.wasm).
5. Com o perfil de assinatura ativo e a CLI do Tizen Studio, gere o pacote assinado:
   - tizen package -t wgt -s NOME_DO_PERFIL -- CAMINHO_DA_PASTA_EXTRAIDA
6. Use Tools > Device Manager para adicionar o IP da TV, estabelecer conexão e instalar o WGT **assinado**. Alternativamente, com a CLI:
   - sdb connect IP_DA_TV
   - sdb devices
   - tizen install -s SERIAL_DA_SAIDA_SDB -n PACOTE_ASSINADO.wgt -- PASTA_DO_PACOTE_ASSINADO
7. Abra **StreamMax Tizen Preview** pela lista de aplicativos.

O serial do parâmetro -s deve corresponder à saída de sdb devices. Não use ADB (Android), APK ou pen drive para esse tipo de instalação Tizen.

## Problemas comuns

- **1010 / erro de assinatura:** conferir certificado Samsung, DUID, validade e perfil de assinatura.
- **TV não detectada:** confirmar Developer Mode, IP, reinicialização, mesma rede e firewall.
- **Tela preta:** conferir compatibilidade Tizen 5.5+, presença de index.wasm, versão do firmware e logs no Tizen Studio.
- **Tela interna parecida com o Nuvio:** esperado nesta versão. O motor Tizen ainda é o do Nuvio Native Legacy; só o invólucro foi identificado como StreamMax.

## Geração, auditoria e atribuição

Workflow em .github/workflows/streammax-tizen-preview.yml. Ele baixa exatamente a release v2.0.8, confere seu SHA-256, altera metadados e ícone, remove assinaturas originais inválidas e gera um novo .wgt unsigned. A GPLv3 original permanece aplicável. Esta automação **não compila o Kotlin Android para Tizen**.

## Aceitação antes do lançamento

Validar modelo e versão Tizen, abertura e navegação pelo controle, login/perfis, séries/filmes, fontes, PT-BR e PT-PT, reprodução, continuar assistindo, memória, desempenho, estabilidade e eventual adaptação de layout e recursos próprios do StreamMax 1.0.11.

## Documentação oficial

- [Samsung TV Device](https://developer.samsung.com/smarttv/develop/getting-started/using-sdk/tv-device.html)
- [Samsung Tizen CLI](https://developer.samsung.com/smarttv/develop/getting-started/using-sdk/command-line-interface.html)
- [Samsung Tizen Studio FAQ](https://developer.samsung.com/smarttv/develop/faq/tizen-studio.html)
