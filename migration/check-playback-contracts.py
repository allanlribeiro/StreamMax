#!/usr/bin/env python3
"""Verificação estática de regressões na retomada; não reproduz vídeo."""
from pathlib import Path
import sys

root = Path(sys.argv[1]) if len(sys.argv) == 2 else Path("legacy-source")
checks = {
    "src/app.c": [
        'fontevolta_esquecer("fonte fora dos filtros atuais")',
        "if (!preparoEspera && tocarFonteGuardada())",
        "stream_folha_abrir();",
        "double posTroca = -1.0, durTroca = 0.0;",
        "player_duracao_midia(&durTroca)",
        "pos < durTroca - 5.0",
        "t == epTrocaT && e == epTrocaE",
        "player_definir_retomada_fonte(posTroca)",
        "streammax_should_ask_first_source(ajustes_fonte_manual(), stream_n(), lembrada)",
        "ultRevisao != fontepref_revisao()",
        "ultManual != manual",
        "ultLembrada = manual ? fontepref_escolher_manual(base) : fontepref_escolher(base);",
        "fontepref_escolher_manual(base)",
        "if (base[0]) fontepref_guardar(base, s);",
    ],
    "src/player.c": [
        "void player_abrir(",
        "armarTituloGuardado(void)",
        "cacheboost_backend_titulo(alvo)",
        "ajustes_retomada_morna()",
        "const char *marca = (!canalSessao && c) ? artehero_logo_sessao(c) : NULL;",
    ],
    "android/app/src/main/java/space/nuvio/nativelegacy/NvPlayer.kt": [
        "p.setMediaItem(item, s.inicioMs.toLong())",
        "p.seekTo(d * f / 10000L)",
    ],
    "src/fontevolta.c": [
        "geracao = confGeracao;",
        "if (geracao == confGeracao) confEstado = FV_NADA;",
        "int fontevolta_toca_guardada(",
        "if (idadeLink >= FONTEVOLTA_VALIDADE_MS) return;",
        "strlen(alvo) >= sizeof eAlvo || strlen(conta) >= sizeof eConta",
        "if (escolherManual || falhaAtiva || canal || !playerAberto) return 0;",
    ],
    "src/fontevolta.h": [
        "FONTEVOLTA_PRAZO_MS",
        "FONTE_MANUAL_PRAZO_MS",
        "fontevolta_decidir(",
    ],
    "src/ajustes.c": [
        "int ajustes_cw_estilo(void)",
        "int ajustes_cw_fonte(void)",
        "int ajustes_fonte_prazo_ms(void)",
        "static void streammaxMigrarPrimeiraFonte(int ajustesExistiam)",
        'const char *const marca = "streammax-primeira-fonte-1018.txt";',
        "if (feito) { free(feito); return; }",
        "valor[AJ_FONTE_MANUAL] = 0;",
        "if (ajustesExistiam && !gravar())",
        'if (!dados_gravar(marca, "1\\n"))',
        "streammaxMigrarPrimeiraFonte(0);",
        "streammaxMigrarPrimeiraFonte(1);",
    ],
    "src/streammax_first_source.h": [
        "static inline int streammax_should_ask_first_source(",
        "return availableStreams > 0 && rememberedIndex < 0;",
        "return !hasManualPreference;",
    ],
    "src/fontepref.h": [
        "int fontepref_escolher_manual(const char *id);",
        "int fontepref_guardar(const char *id, const Stream *s);",
    ],
    "src/ajustes_ux_padrao.inc": [
        "#ifdef NV_ANDROID\n  0,                /* espera pelos add-ons: Instantaneo",
        "#else\n  2,                /* outras plataformas: manter os 5 s",
    ],
    "src/plrui.c": [
        "static void acentoStreamMax(float *r, float *g, float *b)",
        "void plrui_pilula_foco(GfxRect r, float a)",
        "void plrui_barra(float x, float y, float w, float frac",
        "void plrui_disco_osd(GfxRect r, float a)",
        "acentoStreamMax(&ar, &ag, &ab);",
    ],


}
errors = []
for name, needles in checks.items():
    path = root / name
    if not path.is_file():
        errors.append(f"Ausente: {name}")
        continue
    text = path.read_text(encoding="utf-8")
    errors += [f"{name}: falta {needle}" for needle in needles if needle not in text]
    if name == "src/app.c" and '\\n  // lembrada ficou proibida' in text:
        errors.append("src/app.c: comentário com quebra de linha literal inválida")
if errors:
    print("\n".join(errors), file=sys.stderr)
    sys.exit(1)
print("OK: contratos estáticos de retomada, cache e continuar assistindo presentes")
print("PENDENTE: validação funcional com vídeo e login real na TV")
