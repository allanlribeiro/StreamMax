# StreamMax 1.0.5 — Abertura informativa e carregamento experimental (TCL Google TV)

Disponibilizada no atualizador do próprio StreamMax a pedido do usuário em 20/10/2026. Esta versão passou na compilação e nas verificações automatizadas, mas **a melhora real no tempo de início ainda precisa ser testada na TCL**.

## Mudanças

- **Progresso descritivo ao iniciar o vídeo:** a abertura cinematográfica passa a informar os estados conhecidos do aplicativo: procurando fontes; verificando/abrindo pelo debrid; conectando à fonte; recebendo dados; buffer em segundos reais, quando informado pelo player; e iniciando reprodução depois de o vídeo estar preparado, antes do primeiro quadro.
- **Rebuffer informado:** quando a imagem para temporariamente para receber dados, o anel recebe o texto “Recarregando buffer…”.
- **Primeira leitura experimental de MKV/MP4:** o primeiro bloco do mecanismo de download paralelo passou de 1 MiB para 512 KiB. Em fontes progressivas isso pode reduzir o atraso de entrega do primeiro bloco. O comportamento pode variar por servidor e título. **Não foram alterados os limites de memória da TCL, o buffer automático Media3, nem o caminho HLS/DASH.**
- **Mantidos:** Home premium da 1.0.4, formatos de pôster, escolha manual da primeira fonte, perfis, login, idiomas, legendas, player Media3 e atualizador interno.
- Versão StreamMax `1.0.5`, Android `2.0.12` (`versionCode=20012`), superior à `2.0.11` (20011) da 1.0.4.

## Como atualizar sem desinstalar

1. Na TCL, abra o StreamMax **1.0.4**.
2. Entre em **Configurações → Atualizar o aplicativo → Procurar atualização**.
3. Escolha **1.0.5** e **Atualizar agora**, e confirme no instalador do Android TV.
4. **Não desinstale o aplicativo nem limpe dados**. A atualização mantém os dados locais quando o pacote e a assinatura coincidem.

Se o Android TV bloquear a instalação, permita atualizações por esta fonte nas configurações de segurança do sistema; não é necessário fornecer senha do StreamMax.

## Teste recomendado na TCL

Compare **o mesmo filme, mesmo arquivo e mesmo provedor** no Nuvio original e no StreamMax 1.0.5, marcando os segundos entre selecionar a fonte e ver o primeiro quadro. Observe a etapa que permanece mais tempo na tela. Se uma fonte for Torrentio/TorBox, verifique também se consta como disponível em cache. Caso a reprodução fique mais lenta ou falhe em algum formato, reporte para uma revisão; a redução do bloco inicial pode precisar de ajuste.

## Integridade da publicação

- Código da RC1: `2001991fb6c57a12c5ab25a5437ec6546df8164c`
- Build CI: [StreamMax RC1 assinada 152](https://github.com/allanlribeiro/StreamMax/actions/runs/39431785860) — execução `39431785860`, `success`, artefato `StreamMax-RC1-Assinada-152`, ID `12268906760`.
- Baseline assinada da 1.0.4: [StreamMax RC1 151](https://github.com/allanlribeiro/StreamMax/actions/runs/39426041703).
- O publicador verifica origem, pacote `com.streammax.tv`, numeração de versão, validade da assinatura e equivalência do certificado com a RC1 151. Publica **o mesmo APK assinado da RC1 152, sem recompilar nem re-assinar**, acompanhado pelo SHA-256.
