# StreamMax 1.0.11 — protótipo de imagem para fontes dubladas

**Status:** experimental (sem APK oficial, sem OTA, sem alteração da 1.0.10).
**Base:** `feature/streammax-1.0.10-legacy-208` (1.0.10 RC-2 pendente de validação TCL).
**Alvo:** TCL P7K, Android TV de 2 GB RAM. Preservar reprodução 4K, HDR/DV e controles atuais.

## Problema observado
O mesmo vídeo 1080p com arquivo pequeno e dublagem brasileira aparenta mais borrões
e artefatos que uma fonte 1080p de maior tamanho. O YouTube 1080p apresenta boa nitidez
na TCL e a imagem do StreamMax e do Nuvio original é semelhante. As configurações
de nitidez e redução MPEG da TV não resolveram. Dublagem deve continuar prioridade.

## Experimento implementado no código
- `StreamMaxImagePreview.kt`: um shader GLES2 de **uma passagem e cinco amostras**
  suavemente adaptativo a bordas. É **sharpen**, NÃO é IA, aumento real de resolução
  nem recuperação de detalhes inexistentes. Mantém a geometria de entrada.
- `media3-effect:1.8.0`, mesma versão do restante do Media3, por `setVideoEffects`.
- Em produção, **zero mudança de renderização**: o recurso exige aplicativo
  debuggable `com.streammax.tv.strictdebug` E ativação manual no Android.
- A seleção de faixa pode desligar o filtro quando detectar HDR/HLG, DRM,
  mais de 1920×1080 ou mais de 30 fps; tamanho real acima de 1080p também desliga.
  Se houver >=4 quadros perdidos em dois intervalos seguidos, o filtro é retirado.
  *Limitação técnica:* retirar o filtro durante o vídeo não necessariamente
  remove o caminho GL até uma nova sessão. Logo, **não equivale a garantia
  de recuperação de FPS**, precisa comparar on/off no mesmo hardware.
- Se Media3 detectar HDR na criação do shader, usa `PassthroughShaderProgram`.
  Não é uma garantia de pass-through HDR nativo se o pipeline GL já tiver iniciado.
  Não usar HDR/Dolby Vision neste piloto; validar depois que toda a detecção ocorre
  a tempo, ou mudar para uma estratégia sem GL.
- Não modifica seleção de fontes ou preferência de áudio PT-BR; nenhuma alteração IPTV.

## Ativar somente para desenvolvedor

Compilar **debug** e instalar ao lado da versão oficial (pacote
`com.streammax.tv.strictdebug`; necessita ferramentas Android e backend de login).
ADB conectado à TCL:

```sh
adb shell settings put global streammax_image_preview 1
# fechar e reabrir StreamMax strictdebug antes do novo filme SDR 720p/1080p
adb logcat -s NvPlayer:I '*:S'  # procurar [imagem-preview]
adb shell settings delete global streammax_image_preview
```

O ajuste é global da TV e persiste até ser excluído; não guarda dados do filme.
**Desligar novamente após o teste**, especialmente antes de abrir conteúdo 4K/HDR.
A 1.0.10 oficial ignora a chave, pois a verificação exige APK debuggable.

## Portas de qualidade antes de qualquer inclusão definitiva

1. Compilar `compileDebugKotlin` e rodar `StreamMaxImagePreviewTest`;
   as verificações Kotlin/C devem passar e o APK strictdebug não pode substituir a 1.0.10 oficial.
2. Mesma cena/same file em SDR 720p e 1080p, com chave 0 versus 1.
   Medir FPS, quedas de quadro, PSS/memória, temperatura, tempos de abertura, seek.
   Desligar se imagem piorar, se ocorrer erro de GL ou perda de fluidez.
3. Garantir que 4K/HDR/DV usem o caminho nativo **sem inicializar o pipeline GL**.
   O protótipo ainda não pode demonstrar isso, pois dimensões/HDR podem ser
   conhecidos apenas depois do prepare.
4. Guardar capturas da TCL em cenas com textura, rosto e movimento.
   Se melhorar, pensar em controle de usuário no app (Desligado / Suave / Automático),
   por categoria de fonte e opção de desligar instantaneamente. Só então avaliar release.

## Referências
- https://developer.android.com/reference/androidx/media3/exoplayer/ExoPlayer#setVideoEffects(java.util.List)
- https://developer.android.com/reference/androidx/media3/effect/BaseGlShaderProgram
- https://developer.android.com/media/grow/hdr-playback
