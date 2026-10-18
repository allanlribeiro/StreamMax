#!/usr/bin/env python3
"""Validacao estatica do OSD StreamMax: apenas design, sem compilar APK."""
from pathlib import Path
import sys

root = Path(sys.argv[1]) if len(sys.argv) == 2 else Path("legacy-source")
player = (root / "src/player.c").read_text(encoding="utf-8")
plrui = (root / "src/plrui.c").read_text(encoding="utf-8")
header = (root / "src/plrui.h").read_text(encoding="utf-8")
original = Path("app/src/main/java/com/nuvio/tv/ui/screens/player/PlayerScreen.kt")
if not original.exists():
    raise SystemExit("Referencia original nao encontrada no repositorio")
compose = original.read_text(encoding="utf-8")
if "focusedContainerColor = Color.White" not in compose:
    raise SystemExit("Contrato do foco branco do player original mudou")
if "PlayerControlsProgressBarHost(" not in compose:
    raise SystemExit("Barra de progresso do original nao encontrada")

checks = {
    "fundo branco sob foco": "gfx_cor(r, 0.5f, 1.0f, 1.0f, 1.0f, alpha * f)" in player,
    "icones sem pilula expansivel": "streammax_player_controle_diametro(cw, nAcoes" in player,
    "refocalizacao D-pad intacta": "static void transporteMover(" in player,
    "controle de fonte mantido": "(PlrAcao){PLR_FONTES, 1}" in player,
    "controle de episodios mantido": "(PlrAcao){PLR_EPISODIOS, 1}" in player,
    "controle de audio mantido": "(PlrAcao){PLR_AUDIO, 1}" in player,
    "controle de legenda mantido": "(PlrAcao){PLR_CC, 1}" in player,
    "controle de proporcao mantido": "(PlrAcao){PLR_ASPECTO, 1}" in player,
    "barra oficial chamada": "plrui_barra_original(bx, yBarra" in player,
    "barra oficial declarada": "void plrui_barra_original(" in header,
    "barra oficial implementada": "void plrui_barra_original(" in plrui,
    "barra Legacy preservada": "void plrui_barra(" in plrui,
    "cabeça branca exclusiva do Legacy": "if (foco && cabecaBranca)" in plrui,
    "degrade do rodape": "NV_VTELA_H - 400.0f" in player,
    "degrade superior": "NV_VTELA_W, 300.0f" in player,
    "motor de video preservado": "void player_abrir(" in player and "player_duracao_midia(&durTroca)" in (root / "src/app.c").read_text(encoding="utf-8"),
}
section = player[player.index("static void desenharOsdCorpo(Uint32 agora", player.index("void player_desenhar")):]
app = (root / "src/app.c").read_text(encoding="utf-8")
defaults = (root / "src/ajustes_ux_padrao.inc").read_text(encoding="utf-8")
checks["primeira fonte manual por padrao"] = "escolher a fonte ao reproduzir: DESLIGADO" not in defaults and "StreamMax: perguntar a primeira fonte" in defaults
checks["retomada exige preferencia manual"] = "if (!base[0] || !fontepref_tem(base)) return 0;" in app
checks["link retomado casa com a escolha"] = "if (manual && fontepref_escolher_lista(base, &s, 1, NULL, NULL) != 0)" in app
checks["reserva de fonte com perfil"] = "fontevolta_pegar(alvo, sessao_usuario(), perfis_ativo()" in app
checks["sem pilula dinamica na fileira"] = "wCheio" not in section and "plrui_pilula_foco(r" not in section
controls = player.split("static int transporteAcoes(PlrAcao a[PLR_NBTNS]) {", 1)[-1].split("return n;", 1)[0]
order = ["PLR_PLAY", "PLR_PROXIMO", "PLR_CC", "PLR_AUDIO", "PLR_FONTES", "PLR_EPISODIOS", "PLR_ASPECTO", "PLR_REINICIAR"]
spots = [controls.find("(PlrAcao){" + x) for x in order]
checks["ordem dos controles do original primeiro"] = all(p >= 0 for p in spots) and spots == sorted(spots)

failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(("OK" if ok else "FALHOU") + ": " + name)
if failed:
    raise SystemExit("Falhas nos contratos do player: " + ", ".join(failed))
print("Contratos estaticos do novo OSD aprovados. Pendente: build Android e teste TCL.")
