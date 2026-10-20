#!/usr/bin/env python3
"""Verificações estáticas mínimas de login para a árvore Legacy preparada.

Não acessa rede, não recebe credenciais, não compila APK e NÃO prova login real.
Uso: python3 migration/check-legacy-auth.py ../StreamMax-Legacy
"""
from pathlib import Path
import sys

def main(root: Path) -> int:
    required = {
        "src/login.c": [
            "LE_EMAIL", "LE_SENHA", "sessao_login_email",
            "void login_iniciar(void)",
            "static void painelLoginStreamMax(void)",
            "static void loginAcento(float *r, float *g, float *b)",
            '"Bem-vindo ao StreamMax"',
            "teclado_desenhar(agora)",
            'getenv("NUVIO_INTERFACE_AUTO")',
            'if (autoMobile && !strcmp(autoMobile, "0")) return 1;',
            "if (!loginEhTvAndroid()) return;",
            "if (loginEhTvAndroid()) {",
            "logoapp_streammax_simbolo(",
        ],
        "src/sessao.h": [
            "SES_EMAIL", "sessao_login_email", "sessao_login_comecar",
            "sessao_erro_email", "sessao_token_copiar",
        ],
        "src/sessao.c": [
            "sessao_login_email", "sessao_login_comecar", "sessao_sair",
        ],
        "src/xtream.c": [],
        "src/stalker.c": [],
        "src/guia.c": [],
    }
    # Verificações específicas da identidade do StreamMax após a adaptação.
    gradle = root / "android/app/build.gradle.kts"
    app_name = root / "android/app/src/main/res/values/strings.xml"
    settings = root / "src/ajustes.c"
    required.update({
        "android/app/build.gradle.kts": ['applicationId = "com.streammax.tv"'],
        "android/app/src/main/res/values/strings.xml": ['<string name="app_name">StreamMax</string>'],
        "src/ajustes.c": [
            "static int idiomaEfetivo = IDIOMA_PT;",
            "valor[AJ_IDIOMA] = IDIOMA_PT + 1;",
            "if (fonte == IDA_PADRAO) novo = IDIOMA_PT;",
            "if (valor[AJ_IDIOMA] != 0) return;",
        ],
    })
    errors = []
    # StreamMax usa somente email e senha nesta interface.
    login = root / "src/login.c"
    if login.is_file():
        ui = login.read_text(encoding="utf-8")
        forbidden = ("LG_QR", "LE_QR", "gerarTexQr", "sessao_login_comecar()",
                     "Usar o código QR")
        for term in forbidden:
            if term in ui:
                errors.append(f"src/login.c: fluxo QR inesperado: {term}")

    for relative, needles in required.items():
        file = root / relative
        if not file.is_file():
            errors.append(f"Arquivo ausente: {relative}")
            continue
        content = file.read_text(encoding="utf-8")
        for needle in needles:
            if needle not in content:
                errors.append(f"{relative}: símbolo ausente: {needle}")
    if errors:
        for error in errors:
            print("FALHOU:", error, file=sys.stderr)
        return 1
    print("OK: verificações estáticas do login exclusivo por e-mail/senha e IPTV.")
    print("PENDENTE: testes de protocolos, backend, persistência e login real na TV.")
    return 0

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Uso: check-legacy-auth.py CAMINHO-DO-LEGACY", file=sys.stderr)
        sys.exit(2)
    sys.exit(main(Path(sys.argv[1])))
