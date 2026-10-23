#ifndef STREAMMAX_WINDOWS_MPV_H
#define STREAMMAX_WINDOWS_MPV_H
#ifdef NV_WINDOWS_DESKTOP
#include "video.h"
int sm_mpv_init(void);
int sm_mpv_play(const char *url);
void sm_mpv_stop(void);
void sm_mpv_pump(VideoSimulacao *state);
void sm_mpv_pause(int paused);
int sm_mpv_paused(void);
void sm_mpv_seek(double seconds);
void sm_mpv_volume(int percent);
void sm_mpv_window(int x, int y, int w, int h);
void sm_mpv_resize(int w, int h);
void sm_mpv_select_audio(int index);
void sm_mpv_select_subtitle(int index);
void sm_mpv_external_subtitle(const char *url);
void sm_mpv_headers(const char *headers);
void sm_mpv_speed(int percent);
void sm_mpv_composite(void);
void sm_mpv_report_swap(void);
int sm_mpv_failed(void);
int sm_mpv_ended(void);
const char *sm_mpv_error(void);
#endif
#endif
