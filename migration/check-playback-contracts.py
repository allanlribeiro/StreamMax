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
        "rememberedIndex < 0 || rememberedIndex >= availableStreams",
        "return !hasManualPreference;",
    ],
    "src/stream_parse.c": [
        "metadadosFaixas(p, fim, &s);",
        "const char *pistasFonte[] = {nomeFonte, titulo, s.descricao, s.arquivo};",
        'if (audioPista && !s.audioIdiomas[0])',
        'if (legendaPista && !s.legendaIdiomas[0])',
    ],
    "src/guia.c": [
        "logoNaCaixa(c, logoCx",
        "player_mini_no_guia_ativo()",
        "static void streammaxTvDesenhar(",
        "previewPedir(c)",
    ],
    "src/legendasui.c": [
        "static char idiomas[LEGUI_MAX_CAND][16];",
    ],
    "src/abertura.c": [
        "logoapp_streammax_abertura(",
        "AB_SIMBOLO_W",
        "abSaida(p, &veu, &alfaMarca)",
    ],
    "src/main.c": [
        "abertura_fundo_fica(app_no_login() || app_em_escolha_perfil());",
    ],
    "src/streams.c": [
        "stream_faixas_texto(s, legendas, idiomas, sizeof idiomas)",
        'streammax_audio_dual_fonte(s->arquivo, s->nomeFonteCompleto[0] ?',
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
# A nova abertura usa a logo solicitada, estática e sem vídeo ou zoom.
# Verificar a origem identica do PNG no SDL e na janela Android, protegendo
# o primeiro frame contra flash da arte antiga.
icon = root / "deploy/app/art/marcas/streammax_startup.png"
launch = root / "android/app/src/main/res/drawable-nodpi/streammax_startup.png"
if not icon.is_file() or not launch.is_file():
    errors.append("Nova logo não encontrada no SDL ou na janela Android")
elif icon.stat().st_size < 1024 or not icon.read_bytes().startswith(bytes((137, 80, 78, 71, 13, 10, 26, 10))):
    errors.append("PNG da marca StreamMax invalido ou vazio")
elif icon.read_bytes() != launch.read_bytes():
    errors.append("Icone Android nao corresponde aos pixels oficiais da marca SDL")
janela = root / "android/app/src/main/res/drawable/abertura.xml"
if not janela.is_file():
    errors.append("Drawable da abertura nao encontrado")
else:
    import xml.etree.ElementTree as ET
    try:
        a = ET.parse(janela).getroot()
        android_ns = "{http://schemas.android.com/apk/res/android}"
        bitmaps = a.findall("./item/bitmap")
        if (a.tag != "layer-list" or len(bitmaps) != 1
                or bitmaps[0].get(android_ns + "src") != "@drawable/streammax_startup"
                or bitmaps[0].get(android_ns + "gravity") != "fill"):
            errors.append("Janela Android deve mostrar a nova logo completa e centralizada")
        if "@drawable/streammax_launch" in janela.read_text(encoding="utf-8"):
            errors.append("Drawable da abertura ainda referencia JPEG antigo")
    except ET.ParseError:
        errors.append("Drawable de abertura XML invalido")
tema31 = root / "android/app/src/main/res/values-v31/themes.xml"
if not tema31.is_file() or 'name="Tema.StreamMax"' not in tema31.read_text(encoding="utf-8"):
    errors.append("O Android 12+ nao esta usando o tema de abertura StreamMax")
abertura = root / "src/abertura.c"
if abertura.is_file():
    conteudo_abertura = abertura.read_text(encoding="utf-8")
    for antigo in ("streammax_splash.jpg", "logoapp_streammax_wordmark(", "IMG_Load("):
        if antigo in conteudo_abertura:
            errors.append(f"Abertura nativa referencia arte antiga: {antigo}")
    if "veu = q * q * (3.0f - 2.0f * q);" not in conteudo_abertura:
        errors.append("Fade suave da abertura para perfis ausente")
activity = root / "android/app/src/main/java/space/nuvio/nativelegacy/NuvioActivity.kt"
if activity.is_file() and "StreamMaxAberturaVideo" in activity.read_text(encoding="utf-8"):
    errors.append("Activity ainda tenta executar a vinheta MP4 removida")
if (root / "android/app/src/main/java/space/nuvio/nativelegacy/StreamMaxAberturaVideo.kt").exists():
    errors.append("Classe de video antiga nao foi removida")
for name, needles in checks.items():
    path = root / name
    if not path.is_file():
        errors.append(f"Ausente: {name}")
        continue
    text = path.read_text(encoding="utf-8")
    errors += [f"{name}: falta {needle}" for needle in needles if needle not in text]
    if name == "src/app.c" and '\\n  // lembrada ficou proibida' in text:
        errors.append("src/app.c: comentário com quebra de linha literal inválida")
# Evitar regressao: uma palavra de idioma isolada nao comprova Dual Audio.
streams_text = (root / "src/streams.c").read_text(encoding="utf-8") if (root / "src/streams.c").is_file() else ""
if 'temNoTexto(s, "pt-br") || temNoTexto(s, "ptbr"))) return "Dual áudio"' in streams_text:
    errors.append("src/streams.c: PT-BR isolado nao pode ser chamado de Dual Audio")

app_text = (root / "src/app.c").read_text(encoding="utf-8") if (root / "src/app.c").is_file() else ""
if "if (lembradaPreparo >= 0 && !usouPreparo &&" not in app_text:
    errors.append("src/app.c: prefetch pendente pode pular escolha inicial")
if "if (!base[0] || !fontepref_tem(base)) return 0;" not in app_text:
    errors.append("src/app.c: link salvo sem escolha manual pode pular folha")

if errors:
    print("\n".join(errors), file=sys.stderr)
    sys.exit(1)
print("OK: contratos estáticos de retomada, cache e continuar assistindo presentes")
print("PENDENTE: validação funcional com vídeo e login real na TV")
