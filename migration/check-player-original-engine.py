#!/usr/bin/env python3
"""Check StreamMax's first Media3 playback parity step (static, no APK or video)."""
from pathlib import Path
import sys

root = Path(__file__).resolve().parent.parent
source = (root / "legacy-source/android/app/src/main/java/space/nuvio/nativelegacy/NvPlayer.kt").read_text()
original = (root / "app/src/main/java/com/nuvio/tv/ui/screens/player/NuvioExoPlayerPerformanceHelper.kt").read_text()
original_controller = (root / "app/src/main/java/com/nuvio/tv/ui/screens/player/PlayerRuntimeControllerInitialization.kt").read_text()
postplay = (root / "legacy-source/src/posplay.c").read_text()

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
    "back buffer telemetry present": '"teto=${bytes shr 20}MB preparo=$preparando originalStock=$inicioOriginal"' not in source and '"inicio=${inicioMs}ms rebuffer=${recuoBufferMs}ms backBuffer=${backBufferMs}ms "' in source,

    "startup threshold from stock Media3": "DefaultLoadControl.DEFAULT_BUFFER_FOR_PLAYBACK_MS else 2_500" in source,
    "rebuffer threshold from stock Media3": "DefaultLoadControl.DEFAULT_BUFFER_FOR_PLAYBACK_AFTER_REBUFFER_MS else 5_000" in source,
    "prepared stream retains its old threshold": "else parameters.bufferedDurationUs >= 2_500_000L" in source,
    "original ExoPlayer video engine retained": "ExoPlayer.Builder(act, r.renderizadores)" in source,
    "diagnostic startup measurements logged": "originalStock=$inicioOriginal" in source,
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
