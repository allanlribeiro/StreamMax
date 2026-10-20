#!/usr/bin/env bash
# RC1 #142: marca visual apenas em login, perfis e guia TCL.
set -euo pipefail
cd "$(dirname "$0")/.."
python3 - <<'PY'
from pathlib import Path
login=Path("src/login.c").read_text()
perfis=Path("src/perfilsel.c").read_text()
guia=Path("src/guia.c").read_text()
logo=Path("src/logoapp.c").read_text()
for nome in ("streammax_icon.png", "streammax_banner.png"):
    assert (Path("deploy/app/art/marcas")/nome).is_file(), nome
assert "logoapp_streammax_simbolo(" in login
assert "logoapp_streammax_wordmark(" in perfis
assert "logoapp_streammax_wordmark(" in guia
assert 'txt_linha(TXT_G30B, "STREAMMAX"' not in login
assert 'txt_linha(TXT_G30B, "STREAM"' not in guia
assert "if (!login_fundo_desenhar(1.0f))" in login
assert "painelLoginStreamMax();" in login
assert "streammaxPerfilAcabamento(a * (pinDe >= 0 ? 0.60f : 1.0f));" in perfis
assert "streammaxTvDesenhar(a, agoraT);" in guia
assert "SDL_ConvertSurfaceFormat" in logo
print("OK: branding de login/perfis/TV sem alterar player/home/detalhes")
PY
