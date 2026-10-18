#!/usr/bin/env python3
"""Validacao estatica do OSD StreamMax: apenas design, sem compilar APK."""
from pathlib import Path
import sys

root = Path(sys.argv[1]) if len(sys.argv) == 2 else Path("legacy-source")
player = (root / "src/player.c").read_text(encoding="utf-8")
plrui = (root / "src/plrui.c").read_text(encoding="utf-8")
header = (root / "src/plrui.h").read_text(encoding="utf-8")
faixas = (root / "src/faixas.c").read_text(encoding="utf-8")
ilha = (root / "src/plrilha.c").read_text(encoding="utf-8")
ilha_header = (root / "src/plrilha.h").read_text(encoding="utf-8")
legendas = (root / "src/legendasui.c").read_text(encoding="utf-8")
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
checks["referencia original tem seletor independente de legendas"] = (
    "SubtitleSelectionOverlay(" in (Path("app/src/main/java/com/nuvio/tv/ui/screens/player/SubtitleSelectionOverlay.kt")).read_text(encoding="utf-8")
)
checks["legendas e estilo fora da ilha do relogio"] = (
    "p.centro = modo;" in faixas
    and "p.painelEsquerdo = modo;" in faixas
    and "int painelEsquerdo;" in ilha_header
)
checks["cartao de legendas alinhado a esquerda e na base"] = (
    "if (c->painelEsquerdo)" in ilha
    and "R.x = F.x = x;" in ilha
    and "NV_TELA_H - h - margemBaixo" in ilha
    and "NV_TELA_H - c->h - margemBaixo" in ilha
)
checks["audio continua no fluxo anterior, video fica intocado"] = (
    "p.centro = modo;" in faixas
    and "p.painelEsquerdo = modo;" in faixas
    and "else gfx_veu_css(" in faixas
    and "void player_abrir(" in player
)
checks["sobreposicao de legenda preserva recorte e hitbox"] = (
    "cenUlt.painelEsquerdo && painelLegRect.w > 0.0f" in ilha
    and "if (r) *r = painelLegRect;" in ilha
    and "painelLegRect = c->painelEsquerdo ? R" in ilha
)

checks["painel de legendas com colunas independentes"] = (
    "p.w = est ? NV_TELA_W - 192.0f : modo ? 1180.0f : IL_W;" in faixas
    and "desenharIdiomas(x0, y, LU_IDIOMAS_LARG, a);" in legendas
    and "w -= LU_IDIOMAS_LARG + LU_IDIOMAS_ESP;" in legendas
)
checks["coluna de idiomas reage ao controle remoto"] = (
    "if (emIdiomas && !menu)" in legendas
    and "k == SDLK_DOWN && focoIdioma < nIdiomas" in legendas
    and "k == SDLK_RIGHT || k == SDLK_AC_BACK" in legendas
    and "emIdiomas = 1; focarIdiomaAtual();" in legendas
)
checks["idioma filtra opcoes sem selecionar faixa ou audio"] = (
    "static void selecionarIdioma(void)" in legendas
    and 'snprintf(verIdioma, sizeof verIdioma, "%s", idiomas[focoIdioma - 1]);' in legendas
    and "montarLinhas();" in legendas.split("static void selecionarIdioma(void)", 1)[1].split("void legendasui_abrir(void)", 1)[0]
    and "escolher(" not in legendas.split("static void selecionarIdioma(void)", 1)[1].split("void legendasui_abrir(void)", 1)[0]
)
checks["altura do painel acompanha ambas as colunas"] = (
    "alturaOpcoes > alturaIdiomas ? alturaOpcoes : alturaIdiomas" in legendas
    and "if (y < rodapeMin) y = rodapeMin;" in legendas
)
checks["painel de audio continua com tamanho anterior"] = (
    "modo ? 1180.0f : IL_W;" in faixas
)

failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(("OK" if ok else "FALHOU") + ": " + name)
if failed:
    raise SystemExit("Falhas nos contratos do player: " + ", ".join(failed))
print("Contratos estaticos do novo OSD aprovados. Pendente: build Android e teste TCL.")
