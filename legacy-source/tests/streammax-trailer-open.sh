#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
flags=()
if [ -n "${SM_TRAILER_BASELINE:-}" ]; then flags+=(-DSM_TRAILER_BASELINE); fi
if [ "${SM_SANITIZERS:-0}" = 1 ]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
${CC:-cc} -std=gnu11 -O1 -g -ffunction-sections -fdata-sections \
  -DNV_ANDROID -DNV_TOUCH_UI -DSDL_GetTicks=sm_test_ticks \
  -Isrc ${SM_CFLAGS:-$(pkg-config --cflags sdl2)} "${flags[@]}" \
  "${SM_TRAILER_BASELINE:-src/trailer.c}" tests/test_trailer_open.c \
  -Wl,--gc-sections -lm -o "$tmp/open"
"$tmp/open"
python3 - <<'PY'
from pathlib import Path
host=Path('android/app/src/main/java/space/nuvio/nativelegacy/NvPlayer.kt').read_text()
native=Path('src/video_android.c').read_text()
home=Path('src/home.c').read_text()
assert 'trailer_abrir_home(u,' in home
assert 'abrirComJanela' in native and '(Ljava/lang/String;Ljava/lang/String;IIIIIII)V' in native
assert 'private fun abrirPosicaoInterno' in host
enqueue=host[host.index('private fun abrirPosicaoInterno'):host.index('@JvmStatic fun abrirRetomada')]
assert enqueue.index('janelaPendente.offer') < enqueue.index('abrirMain(')
opening=host[host.index('private fun abrirMain('):host.index('private fun liberar()')]
assert opening.index('player = p') < opening.index('aplicarJanelaPendente(pedido)') < opening.index('p.setVideoSurfaceView(sv)')
assert 'if (!temJanela) aplicarEncaixe()' in opening
assert 'if (pedido == pedidos.get()) definirJanela' in host
assert 'janelaPendente.offer(pedido,x,y,w,h,encaixa)' in host
print('OK: janela acompanha URL/geracao e e aplicada antes da Surface; pedidos antigos sao filtrados')
PY
