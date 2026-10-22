#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 - <<'PY'
from pathlib import Path
src = Path("src/main.c").read_text(encoding="utf-8")
vid = Path("src/video.c").read_text(encoding="utf-8")
mpv = Path("src/windows_mpv.c").read_text(encoding="utf-8")
build = Path("tools/windows.sh").read_text(encoding="utf-8")
bundle = Path("tools/windows-bundle-dlls.sh").read_text(encoding="utf-8")
installer = Path("installer/streammax-windows.iss").read_text(encoding="utf-8")
workflow = Path("../.github/workflows/streammax-windows-experimental.yml").read_text(encoding="utf-8")
art = Path("tools/windows-art.sh").read_text(encoding="utf-8")
icon = Path("tools/windows-icon.py").read_text(encoding="utf-8")

assert "SDL_WINDOW_RESIZABLE" in src
assert "SDL_SetWindowMinimumSize(win, 960, 540)" in src
assert "SDLK_F11" in src and "SDL_WINDOW_FULLSCREEN_DESKTOP" in src
assert "SDLK_ESCAPE || e.key.keysym.sym == SDLK_BACKSPACE" in src
assert "gfx_tamanho_alvo(dw, dh)" in src
assert "ponteiro_evento(&e, app_evento)" in src
assert "sm_mpv_composite()" in src and "sm_mpv_report_swap()" in src
for prop in ("sm_mpv_play(", "sm_mpv_pump(", "sm_mpv_select_audio(", "sm_mpv_select_subtitle(",
             "sm_mpv_external_subtitle(", "sm_mpv_seek(", "sm_mpv_resize("):
    assert prop in vid, prop
for prop in ("mpv_render_context_render", "MPV_RENDER_PARAM_OPENGL_FBO",
             "glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA,GL_ONE", "MPV_EVENT_FILE_LOADED",
             "mpv_command(smPlayer,cmd)", "http-header-fields"):
    assert prop in mpv, prop

assert Path("deploy/app/art/marcas/streammax_icon.png").is_file()
assert "streammax_icon.png" in icon
assert "windows-icon.py" in build and "windres" in build
assert "mpv" in build and "libmpv-2.dll" in bundle
assert "SetupIconFile=" in installer
assert "windows-art.sh" in workflow
assert "libmpv-2.dll" in workflow
for disallowed in ("cp -r deploy/app/art build/windows/package/art",
                   "rm -rf deploy/app/art", "cp -r deploy/app/art/collections"):
    assert disallowed not in workflow
for forbidden in ("*.txt", "*.bin", "*.tmp", "collections", "cache"):
    assert forbidden in art
print("OK: mouse, teclado, player libmpv, icone oficial e pacote seguro")
PY
