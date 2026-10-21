#!/usr/bin/env python3
"""Guarda contra regressões de memória/UX do IPTV Xtream (sem rede)."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
ui = (root / "src/streammax_iptv_vod.c").read_text(encoding="utf-8")
api = (root / "src/xtream.c").read_text(encoding="utf-8")

def require(condition, message):
    if not condition:
        raise AssertionError(message)

caps = {}
for name in ("VOD_CAT_CAP", "VOD_ITEM_CAP", "VOD_EP_CAP"):
    match = re.search(r"^#define\s+" + name + r"\s+(\d+)\s*$", ui, re.M)
    require(match is not None, f"Limite ausente: {name}")
    caps[name] = int(match.group(1))
    require(0 < caps[name] <= 128, f"Limite exagerado: {name}")

require("if (visivel->nItens >= VOD_ITEM_CAP)" in ui,
        "Falta aviso de possível truncamento de títulos")
require("pode haver mais no provedor" in ui,
        "Mensagem de possível truncamento foi removida")
require("if (t->categoria[0] || t->listarTodos)" in ui,
        "VOD não deve baixar todos os títulos na primeira abertura")
require("xtream_vod_listar_categoria" in api, "Consulta por categoria ausente")
require("xtream_vod_categoria_valida(categoriaId)" in api,
        "Falta validar ID da categoria antes da rede")
require("pthread_mutex_lock(&travaRede)" in api,
        "Serialização de chamadas Xtream ausente")
require("while (p && n < cap)" in api,
        "Parser Xtream perdeu o limite de cópia")
require("tex_obter_larg(visivel->itens[i].poster, card.w - 12.0f)" in ui,
        "Capas IPTV devem usar decode proporcional ao card no cache existente")
require("for (int j = 0; j < colunas * linhas; j++)" in ui,
        "Capas devem ser solicitadas apenas para a janela visivel")
require("const int colunas = 4, linhas = 3;" in ui,
        "Orcamento da janela visivel deve continuar em 12 cards")
require("const int maxPrimeiraLinha = totalLinhas > linhas ? totalLinhas - linhas : 0;" in ui,
        "Rolagem nao deve ultrapassar a ultima janela")
require("if (d > 0 && proximo >= itensNaCategoria()" in ui,
        "Navegacao deve alcancar a ultima fileira incompleta")
print("OK: limites IPTV, aviso de truncamento, lazy loading e validação preservados")
