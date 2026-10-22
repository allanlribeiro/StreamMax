#!/usr/bin/env python3
"""Checks for targeted Nuvio Legacy 2.0.8 backports in StreamMax 1.0.10.
These are source contracts; they do NOT replace an Android build or TCL tests.
"""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
activity = (root / "android/app/src/main/java/space/nuvio/nativelegacy/NuvioActivity.kt").read_text()
apks = (root / "android/app/src/main/java/space/nuvio/nativelegacy/StreamMaxApkCache.kt").read_text()
texture = (root / "src/tex_cache.c").read_text()
version = (root / "src/streammax_update.h").read_text()

# Lifecycle transitions must not evict artwork when Android emits a background
# lifecycle trim on a visible screen.
assert "private var trimResumida = false" in activity
assert "if (trimResumida && level >= 20) return" in activity
assert re.search(r"override fun onResume\(\).*?trimResumida = true.*?super.onResume", activity, re.S)
assert re.search(r"override fun onPause\(\).*?trimResumida = false", activity, re.S)
assert activity.index("if (trimResumida && level >= 20) return") < activity.index("SDLActivity.nativeLowMemory()")

# Pressure cap may not evict artwork belonging to the current frame.
limiter = texture[texture.index("static long limiteBytes(void)"):texture.index("int tex_pressao_play_pct(void)")]
assert "if (quente(&itens[i])) tela += bytesTextura(&itens[i]);" in limiter
assert "if (tela > lim) lim = tela;" in limiter

# StreamMax uses private filesDir/dados/atualizacao, NOT the upstream cacheDir.
assert 'File(filesDir, "dados/atualizacao")' in activity
assert "podarApksAntigos()" in activity
assert '"StreamMax-Apk-Prune"' in activity
assert "StreamMaxApkCache.prune(dir, codigo, null)" in activity
assert "archive == null || archive.packageName != packageName" in activity
assert "val versions = apks.associateWith" in apks
assert "if (apk.name == protectedName) continue" in apks
assert "val v = versions[apk] ?: continue" in apks
assert "if (v <= installed || (newest != null && apk != newest)) apk.delete()" in apks
assert "deleteRecursively" not in apks

# Subtitle reuse keeps one cached download for the current playback session,
# and resets the cache at the next play. Downloaded bytes stay owned by cache.
subtitle = (root / "src/legenda.c").read_text()
assert "static PreDownload *downloadsSessao;" in subtitle
assert "static PreDownload *downloadSessao(const char *url)" in subtitle
assert "while (downloadsSessao)" in subtitle
assert "p->refs = 3;" in subtitle
assert "p->pre = downloadSessao(url);" in subtitle
assert "bruto = p->pre->bruto ? malloc((size_t)nb + 1) : NULL;" in subtitle
assert "p->pre->bruto = NULL;" not in subtitle
assert "if (!strcmp(p->url, url) && (!p->terminou || p->bruto))" in subtitle

# Playback checkpoints: save every 30 s locally and on Android lifecycle
# transitions; keep account syncing at a separate 5-minute interval.
player = (root / "src/player.c").read_text()
android = (root / "src/android.c").read_text()
assert "#define PLR_PARCIAL_INTERVALO_S 30.0f" in player
assert "gravarProgressoParcial(parcialConta >= 300.0f" in player
assert 'gravarProgressoParcial(tipo != SDL_APP_LOWMEMORY, "background");' in player
assert "parcialRelogio = parcialConta = 0.0f;" in player
assert "static float parcialRelogio, parcialConta;" in player
assert "Java_space_nuvio_nativelegacy_NuvioActivity_nativeCheckpoint" in android
assert "SDL_PushEvent(&e);" in android
assert "private external fun nativeCheckpoint()" in activity
assert "checkpoint()" in activity
assert "if (apk.name == protectedName) continue" in apks

# The candidate is v1.0.10 on an isolated branch; no public GitHub release is created.
assert '#define SM_UPDATE_VERSION "1.0.10"' in version
assert "apkOficialParaAtualizar(arq)" in activity
assert "oficiais.isNotEmpty() && oficiais == recebidos" in activity
print("PASS: 1.0.10 Android trim, visible artwork, safe OTA cleanup and signed-updater contracts")
