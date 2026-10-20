#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 - <<'PY'
from pathlib import Path
player = Path("src/player.c").read_text(encoding="utf-8")
net = Path("android/app/src/main/java/space/nuvio/nativelegacy/ParaleloDataSource.kt").read_text(encoding="utf-8")
kotlin = Path("android/app/src/main/java/space/nuvio/nativelegacy/NvPlayer.kt").read_text(encoding="utf-8")
assert 'static const char *streammaxEtapaInicio(' in player
assert 'if (esperandoFonte)' in player.split('static const char *streammaxEtapaInicio(',1)[1].split('// A TELA CINEMA',1)[0]
for text in ('Procurando fontes…','Iniciando reprodução…','Carregando buffer: %.1f s',
             'Recebendo dados da fonte…','Conectando à fonte…','Recarregando buffer…'):
    assert text in player, text
assert 'if (video_tocando() && !video_quadro_visto())' in player
assert 's.bufferSeg >= 0.5' in player
assert 's.bytes > 0 || s.primeiroByteMs >= 0' in player
assert 'txt_desenhar_alpha(l, canto ? tx : cx - l.w * 0.5f' in player
assert 'private const val PRIMEIRO = 512 shl 10' in net
assert 'private const val PEDACO_CAUDA_MIN = 128 shl 10' in net
assert 'const val PEDACO = 4 shl 20' in net
assert 'val primeiroUltimo = pos + (if (pedido != C.LENGTH_UNSET.toLong()) minOf(pedido, PRIMEIRO.toLong())' in net
assert 'fun serve(uri: Uri): Boolean' in net and 'p.endsWith(".m3u8")' in net
assert 'if (inicioOriginal)' in kotlin and 'DefaultLoadControl.DEFAULT_BUFFER_FOR_PLAYBACK_MS' in kotlin
print("OK: estados de abertura verificaveis, rebuffer rotulado, sondagem 512 KiB e buffer Media3 preservado")
PY
