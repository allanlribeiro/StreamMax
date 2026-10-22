#!/usr/bin/env python3
"""Static safety contracts for experimental 1.0.11 sharpening. NOT a frame-rate test."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
main = (root / "android/app/src/main/java/space/nuvio/nativelegacy/NvPlayer.kt").read_text()
effect = (root / "android/app/src/main/java/space/nuvio/nativelegacy/StreamMaxImagePreview.kt").read_text()
gradle = (root / "android/app/build.gradle.kts").read_text()
assert 'implementation("androidx.media3:media3-effect:1.8.0")' in gradle
assert 'implementation("androidx.media3:media3-exoplayer:1.8.0")' in gradle
assert "ApplicationInfo.FLAG_DEBUGGABLE" in effect
assert 'Settings.Global.getInt(context.contentResolver, SETTING, 0) == 1' in effect
assert "if (StreamMaxImagePreview.requested(act))" in main
assert "setVideoEffects(StreamMaxImagePreview.effect())" in main
assert "setVideoEffects(emptyList())" in main
assert "C.COLOR_TRANSFER_ST2084" in effect
assert "C.COLOR_TRANSFER_HLG" in effect
assert "drm" in effect and "4k" in effect and "high_fps" in effect
assert "PassthroughShaderProgram()" in effect
assert "texturePoolCapacity= */ 1" in effect
assert effect.count("texture2D(uTexSampler,") == 5
assert "if (imagemPreviewAmostrasRuins >= 2)" in main
assert "checarImagemPreview(tracks)" in main
assert "desligarImagemPreview(it, \"resolucao\")" in main
# Critical: the work is on a prototype branch; never bump a public OTA version.
assert '#define SM_UPDATE_VERSION "1.0.10"' in (root / "src/streammax_update.h").read_text()
print("OK: StreamMax 1.0.11 debug-only image-preview contracts")
