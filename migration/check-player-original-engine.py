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
    "stock threshold only for unprepared automatic VOD": "val inicioOriginal = !preparando && preset == 0" in source,
    "original stock back buffer retains 1.5s for short rewind": ".setBackBuffer(1_500, /* retainBackBufferFromKeyframe = */ true)" in original_controller,
    "streammax keeps 1.5s back buffer on stock VOD": "val backBufferMs = if (inicioOriginal) 1_500 else 0" in source,
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
