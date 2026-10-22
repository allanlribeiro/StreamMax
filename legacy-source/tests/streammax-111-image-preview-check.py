#!/usr/bin/env python3
"""Structural integration checks. Kotlin policy tests and real TV testing are also required."""
from pathlib import Path
r = Path(__file__).resolve().parents[1]
k = r / 'android/app/src/main/java/space/nuvio/nativelegacy'
main = (k/'NvPlayer.kt').read_text()
renderer = (k/'StreamMaxImageRenderer.kt').read_text()
effect = (k/'StreamMaxImagePreview.kt').read_text()
assert 'setVideoEffects(' not in main, 'Do not eagerly initialize GL for unclassified content'
assert 'shouldActivate(formats' in renderer
assert renderer.index('if (eligible)') < renderer.index('video.setVideoEffects(') < renderer.index('super.enable(')
assert 'formats.forEach(::guard)' in renderer
assert 'formatHolder.format?.let(::guard)' in renderer
assert 'override fun maybeInitializeProcessingPipeline' in renderer
assert 'override fun onOutputFormatChanged' in renderer
assert 'StreamMaxImageBypass' in renderer and 'StreamMaxImageBypass' in main
assert 'PassthroughShaderProgram' not in effect, 'HDR must never fall back to HDR-through-GL'
assert 'transfer != C.COLOR_TRANSFER_SDR' in effect
assert 'COLOR_SPACE_BT2020' in effect and 'video/dolby-vision' in effect
assert all(x in effect for x in ('dvhe', 'dvh1', 'dvav', 'dva1'))
assert 'private var imagemAutomatica = false' in main
assert 'imagemBloqueadaNestaFonte = true' in main
assert 'abrirMain(url, cab, true, inicio, geracao, pedido, fracao)' in main
assert 'novo.trackSelectionParameters = parametros' in main
assert 'novo.playbackParameters = velocidade' in main
assert 'soltarPreparo() // Never adopt' in main
assert 'THERMAL_STATUS_MODERATE' in effect
assert 'return badWindows >= 2' in effect
assert 'texturePoolCapacity= */ 1' in effect
assert effect.count('texture2D(uTexSampler,') == 5
assert 'return Size(inputWidth, inputHeight)' in effect
assert 'ajustes_image_upscaling()' in (r/'src/video_android.c').read_text()
assert 'streammaxImageUpscalingLocal' in (r/'src/ajustes.c').read_text()
assert '#define SM_UPDATE_VERSION "1.0.11"' in (r/'src/streammax_update.h').read_text()
print('PASS: guarded HD admission, native HDR/DV/UHD path, OFF default, full fallback and settings integration')
