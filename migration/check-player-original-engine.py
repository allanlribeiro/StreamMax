#!/usr/bin/env python3
"""Check StreamMax's first Media3 playback parity step (static, no APK or video)."""
from pathlib import Path
import sys

root = Path(__file__).resolve().parent.parent
source = (root / "legacy-source/android/app/src/main/java/space/nuvio/nativelegacy/NvPlayer.kt").read_text()
original = (root / "app/src/main/java/com/nuvio/tv/ui/screens/player/NuvioExoPlayerPerformanceHelper.kt").read_text()
original_controller = (root / "app/src/main/java/com/nuvio/tv/ui/screens/player/PlayerRuntimeControllerInitialization.kt").read_text()
original_watchdog = (root / "app/src/main/java/com/nuvio/tv/ui/screens/player/PlayerFirstFrameWatchdogPolicy.kt").read_text()
postplay = (root / "legacy-source/src/posplay.c").read_text()
source_filter = (root / "legacy-source/src/fonteregra.c").read_text()
streams = (root / "legacy-source/src/streams.c").read_text()
preferences = (root / "legacy-source/src/fontepref.c").read_text()

activity = (root / "legacy-source/android/app/src/main/java/space/nuvio/nativelegacy/NuvioActivity.kt").read_text()

# Pausar o app pelo Android nunca pode ser confundido com transicao READY:
# o watchdog nao deve ligar playWhenReady com a Activity em background.
pausa_sistema = source.split("fun pausarPeloSistema() {", 1)[1].split("\n    }", 1)[0]

checks = {
    "original Nuvio performance buffer min 15 s": "DEFAULT_NUVIO_MIN_BUFFER_MS = 15_000" in original,
    "original Nuvio performance buffer max 45 s": "DEFAULT_NUVIO_MAX_BUFFER_MS = 45_000" in original,
    "original stock Media3 load control exists": "DefaultLoadControl.Builder()" in original_controller,
    "legacy automatic buffer uses original 15-45s window": "else Triple(15_000, 45_000, tetoBuffer)" in source,
    "legacy user preset buffer untouched": "Triple(50_000, 150_000, minOf(preset.toLong() shl 20, heap / 3).toInt())" in source,
    "low-RAM byte cap untouched": "val tetoBuffer = if (poucaRam)" in source,
    "stock threshold for cold and adopted prepared VOD": "val inicioOriginal = preset == 0" in source,
    "prepared playback keeps 2.5s threshold before adoption": "else parameters.bufferedDurationUs >= 2_500_000L" in source,
    "adopted preload switches to inherited stock Media3 playback thresholds": (
        "if (adotada) super.shouldStartPlayback(parameters)" in source
        and "carga.adotada = true" in source
    ),
    "original stock back buffer retains 1.5s for short rewind": ".setBackBuffer(1_500, /* retainBackBufferFromKeyframe = */ true)" in original_controller,
    "cold stock VOD keeps 1.5s back buffer": "val backBufferMs = if (inicioOriginal && !preparando) 1_500 else 0" in source,
    "prepared stock VOD does not enlarge its back-buffer memory": "val backBufferMs = if (inicioOriginal && !preparando) 1_500 else 0" in source,
    "back buffer uses keyframe retention only when enabled": "backBufferMs, backBufferMs > 0) {" in source,
    "stock VOD forwards retention setting to load control": "CargaPlayer(minMs, maxMs, bytes, preparando, inicioMs, recuoBufferMs, backBufferMs)" in source,
    "back buffer telemetry present": '"inicio=${inicioMs}ms rebuffer=${recuoBufferMs}ms backBuffer=${backBufferMs}ms "' in source,

    "startup threshold from stock Media3": "DefaultLoadControl.DEFAULT_BUFFER_FOR_PLAYBACK_MS else 2_500" in source,
    "rebuffer threshold from stock Media3": "DefaultLoadControl.DEFAULT_BUFFER_FOR_PLAYBACK_AFTER_REBUFFER_MS else 5_000" in source,
    "prepared stream retains its old threshold": "else parameters.bufferedDurationUs >= 2_500_000L" in source,
    "original ExoPlayer video engine retained": "ExoPlayer.Builder(act, r.renderizadores)" in source,
    "diagnostic startup measurements logged": "originalStock=$inicioOriginal" in source,
    "rebuffer diagnostics only after first frame": "state == Player.STATE_BUFFERING && quadroVisto && p.playWhenReady" in source,
    "manual pause ends rebuffer accounting": "if (!playWhenReady) {\n                encerrarRebuffer()" in source,
    "rebuffer timings reset on new playback": "rebufferInicioEm = 0L\n        rebufferTotalMs = 0L\n        rebufferCount = 0" in source,
    "first frame timing retained": "onRenderedFirstFrame renderer=video loadMs=" in source,
    "original first frame ready watchdog available": "ForcePlayWhenReady" in original_watchdog,
    "StreamMax watchdog checks READY without automatic play": "p.playbackState == Player.STATE_READY && !p.playWhenReady" in source,
    "watchdog checks selected video track": "(0 until g.length).any { g.isTrackSelected(it) }" in source,
    "bitmap subtitle renderer dependency present": (
        'implementation("androidx.media3:media3-ui:1.8.0")' in
        (root / "legacy-source/android/app/build.gradle.kts").read_text()
    ),
    "bitmap subtitle renderer uses existing video layer": (
        "import androidx.media3.ui.SubtitleView" in source
        and "c.addView(view, lp)" in source
        and "view.isFocusable = false" in source
        and "legendaBitmapView?.let { view ->" in source
    ),
    "SSA/ASS cues preserve styled Media3 rendering": (
        "MimeTypes.TEXT_SSA" in source
        and "val imagens = if (usaAss) cueGroup.cues else cueGroup.cues.filter { it.bitmap != null }" in source
        and 'val texto = if (usaAss) "" else cueGroup.cues.mapNotNull' in source
        and "mostrarLegendaBitmap(imagens)" in source
    ),
    "SSA/ASS is selected only by the active text track": (
        "g.type == C.TRACK_TYPE_TEXT && (0 until g.length).any { i ->" in source
        and "g.isTrackSelected(i) && g.getTrackFormat(i).sampleMimeType == MimeTypes.TEXT_SSA" in source
    ),
    "bitmap cues do not replace ordinary native text subtitles": (
        'val imagens = if (usaAss) cueGroup.cues else cueGroup.cues.filter { it.bitmap != null }' in source
        and 'val texto = if (usaAss) "" else cueGroup.cues.mapNotNull' in source
        and "nativeLegenda(texto, dur)" in source
        and "mostrarLegendaBitmap(imagens)" in source
    ),
    "bitmap cues share delayed subtitle generation guard": (
        "if (atual(minha) && versao == versaoLegenda) {" in source
        and "mostrarLegendaBitmap(imagens)" in source
        and "principal.postDelayed(entregar, atrasoMs.toLong())" in source
    ),
    "bitmap cues reset on source teardown, seek and track change": (
        "removerLegendaBitmap()" in source
        and source.count("limparLegendaBitmap()") >= 4
        and "reason != Player.DISCONTINUITY_REASON_SEEK" in source
    ),
    "subtitle track changes invalidate stale cues only when selection changes": (
        "if (tipo == 1 && novosParametros != p.trackSelectionParameters)" in source
        and 'nativeLegenda("", 0)' in source
    ),
    "subtitle delay changes clear stale cues without affecting unchanged settings": (
        "private fun escolherMain(tipo: Int, idx: Int) {" in source
        and (lambda block: (
            "if (atrasoMs != idx) {" in block
            and "versaoLegenda++" in block
            and 'nativeLegenda("", 0)' in block
            and "limparLegendaBitmap()" in block
            and "atrasoMs = idx" in block
        ))(source.split("private fun escolherMain(tipo: Int, idx: Int) {", 1)[1]
                  .split("if (tipo == 3)", 1)[0])
    ),
    "old subtitle cues are rejected after track or delay changes": "atual(minha) && versao == versaoLegenda" in source,
    "seek invalidates pending subtitle cues and clears overlay": (
        "override fun onPositionDiscontinuity(" in source
        and "reason != Player.DISCONTINUITY_REASON_SEEK" in source
        and source.split("override fun onPositionDiscontinuity(", 1)[1]
            .split("override fun onTimelineChanged(", 1)[0].count("versaoLegenda++") == 1
        and 'nativeLegenda("", 0)' in source.split("override fun onPositionDiscontinuity(", 1)[1]
            .split("override fun onTimelineChanged(", 1)[0]
    ),
    "surface timeout recovery is attempted only once per session": (
        "p != null && !recuperouSuperficie && RecriaHdr.timeoutRecuperavel(" in source
        and "recuperouSuperficie = true" in source
        and "recuperouSuperficie = false" in source
    ),
    "surface recovery exceptions report playback error instead of killing UI": (
        'val sv = superficie ?: throw IllegalStateException("video surface unavailable")' in source
        and "catch (retryError: Exception)" in source
        and "ev(EV_ERRO, error.errorCode, C.TRACK_TYPE_VIDEO)" in source
    ),
    "HLS/DASH updates track selection references even with the same displayed labels": (
        "if (nova == assinatura) {\n            // HLS/DASH" in source
        and "audios = a\n            legendas = l\n            return\n        }" in source
    ),
    "HLS/DASH refresh avoids native subtitle menu reset": (
        source.index("if (nova == assinatura) {") < source.index("val tinhaLegenda = legendas.isNotEmpty()")
        and source.index("val tinhaLegenda = legendas.isNotEmpty()") < source.index("nativeFaixasFim(sel, -1)")
    ),
    "Android onPause gates a late source open": (
        "atividadeOculta = true" in pausa_sistema
        and "if (!reabrindo && !atividadeOculta) pausaPedida = false" in source
    ),
    "late source open cannot start in the background": (
        source.count("p.playWhenReady = !pausaPedida && !atividadeOculta") == 3
    ),
    "manual play cannot override Android background pause": (
        "player?.playWhenReady = !pausaPedida && !atividadeOculta" in source
    ),
    "first-frame READY watchdog respects Android lifecycle": (
        "p != null && !quadroVisto && !pausaPedida && !atividadeOculta" in source
        and "if (!quadroVisto && !pausaPedida && !atividadeOculta)" in source
    ),
    "onResume re-enables player without forcing playback": (
        "NvPlayer.retomarPeloSistema()" in activity
        and "atividadeOculta = false" in source
    ),
    "system pause disables READY watchdog before pausing": (
        "pausaPedida = true" in pausa_sistema
        and "principal.removeCallbacks(vigiaPrimeiroQuadro)" in pausa_sistema
        and "player?.pause()" in pausa_sistema
    ),
    "watchdog never overrides manual pause": "p != null && !quadroVisto && !pausaPedida" in source,
    "watchdog is removed after first frame and on release": source.count("principal.removeCallbacks(vigiaPrimeiroQuadro)") >= 3,
    "prepared sessions obey user pause": source.count("p.playWhenReady = !pausaPedida") >= 3,
    "no false paused events without explicit user pause": "if (!playWhenReady && pausaPedida && !p.isPlaying" in source
        and "else if (!p.playWhenReady && pausaPedida && p.playbackState" in source,
    # Audio/language source filtering is a deliberate StreamMax feature and
    # MUST survive player parity work. These are static guards, not playback tests.
    "source filter exposes language presets": "static const char *const MODELOS[]" in source_filter,
    "source filter keeps Portuguese recognition": '"PT-?BR|Portugu|Dublado|' in source_filter,
    "source filter keeps English recognition": '"\\\\bENG?\\\\b|English|' in source_filter,
    "source filter keeps DUAL-labelled streams": '"\\\\bDUAL\\\\b"' in source_filter,
    "source filter keeps non-destructive preference vs scope": "int fonteregra_grupo(" in source_filter and "int fonteregra_no_escopo(" in source_filter,
    "audio filter remains in source picker": "BT_DUB" in streams and "soDub && !temAudioPreferido(&lista[i])" in streams,
    "audio filter is independently selectable": 'soDub == 1 ? "pt" : soDub == 2 ? "en" : ""' in streams and "soDub = (soDub + 1) % 4;" in streams,
    "explicit addon audio metadata wins over guesses": "if (s->audioIdiomas[0]) return 0;" in streams,
    "Portuguese fallback never reads subtitle metadata": "streammax_pt_audio_sem_declaracao(s->arquivo, s->rotulo, s->descricao)" in streams,
    "audio chips do not override full player source filter": "if (!canalFolha && !stream_filtro_permite(&lista[i])) return 0;" in streams,
    "remembered binge group respects audio signature": (
        "static int trilhaCompativel(const FontePref *p, const Stream *s)" in preferences
        and preferences.count("if (!trilhaCompativel(p, s)) continue;") == 2
        and "return contemMarcas(trilha, p->trilha);" in preferences
    ),
    "manual source blocks unintended next-video preloading": "streammax_next_video_preload(preloadVideo, manual)" in postplay,
    "manual source permits metadata-only next prefetch": "streammax_next_metadata_prefetch(" in postplay,
    "metadata prefetch requires remembered manual source": "fontepref_tem(ci->imdb)" in postplay,

}
errors = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(("OK" if ok else "FAIL") + ": " + name)
if errors:
    sys.exit(1)
print("Static Media3 player parity checks passed; Android build and TCL playback remain unverified.")
