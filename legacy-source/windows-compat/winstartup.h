#ifndef STREAMMAX_WINDOWS_STARTUP_DIAG_H
#define STREAMMAX_WINDOWS_STARTUP_DIAG_H
#ifdef NV_WINDOWS_DESKTOP
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Relatorio de etapas, sem URLs, senhas ou dados de login.
// Registrado ANTES de SDL_Init e salvo fora da pasta de instalacao.
static char sm_win_diag_path[1024];
static char sm_win_log_path[1024];
static char sm_win_stage[128];

static void sm_win_mark(const char *etapa) {
  if (!etapa) return;
  snprintf(sm_win_stage, sizeof sm_win_stage, "%s", etapa);
  if (!sm_win_diag_path[0]) return;
  FILE *f = fopen(sm_win_diag_path, "a");
  if (!f) return;
  fprintf(f, "%llu ms: %s\n", (unsigned long long)GetTickCount64(), etapa);
  fclose(f);
}

static LONG WINAPI sm_win_unhandled(EXCEPTION_POINTERS *ex) {
  char b[256];
  unsigned long codigo = ex && ex->ExceptionRecord ?
      (unsigned long)ex->ExceptionRecord->ExceptionCode : 0;
  snprintf(b, sizeof b, "Falha nativa 0x%08lX na etapa: %.100s", codigo, sm_win_stage);
  sm_win_mark(b);
  return EXCEPTION_CONTINUE_SEARCH;
}

static void sm_win_startup(void) {
  char dir[850], anterior[1024];
  const char *base = getenv("LOCALAPPDATA");
  if (!base || !*base) base = getenv("USERPROFILE");
  if (!base || !*base) return;
  if (snprintf(dir, sizeof dir, "%s\\StreamMax", base) >= (int)sizeof dir) return;
  CreateDirectoryA(dir, NULL);
  snprintf(sm_win_diag_path, sizeof sm_win_diag_path, "%s\\diagnostico-inicio.txt", dir);
  snprintf(sm_win_log_path, sizeof sm_win_log_path, "%s\\nuvio.log", dir);
  snprintf(anterior, sizeof anterior, "%s\\nuvio-anterior.log", dir);
  _putenv_s("NUVIO_LOG", sm_win_log_path);
  _putenv_s("NUVIO_LOG_ANTERIOR", anterior);
  { FILE *f = fopen(sm_win_diag_path, "w");
    if (f) { fputs("StreamMax Windows W0 - etapas da inicializacao (sem dados pessoais)\n", f); fclose(f); }
  }
  SetUnhandledExceptionFilter(sm_win_unhandled);
  sm_win_mark("main");
}

static void sm_win_fatal(const char *etapa, const char *motivo) {
  char b[1800];
  sm_win_mark(etapa);
  snprintf(b, sizeof b,
           "O StreamMax nao conseguiu iniciar sua interface.\n\n"
           "Etapa: %s\nMotivo: %s\n\n"
           "Envie o arquivo de diagnostico:\n%s",
           etapa ? etapa : "desconhecida", motivo ? motivo : "sem detalhes",
           sm_win_diag_path[0] ? sm_win_diag_path : "%LOCALAPPDATA%\\StreamMax\\diagnostico-inicio.txt");
  MessageBoxA(NULL, b, "StreamMax - diagnostico de inicializacao", MB_OK | MB_ICONERROR);
}

#endif
#endif
