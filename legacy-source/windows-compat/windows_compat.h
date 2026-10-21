#ifndef STREAMMAX_WINDOWS_COMPAT_H
#define STREAMMAX_WINDOWS_COMPAT_H
#ifdef NV_WINDOWS_DESKTOP

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#include <io.h>
#include <direct.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static inline struct tm *nv_localtime_r(const time_t *t, struct tm *out) {
  return localtime_s(out, t) == 0 ? out : NULL;
}
static inline struct tm *nv_gmtime_r(const time_t *t, struct tm *out) {
  return gmtime_s(out, t) == 0 ? out : NULL;
}
#define localtime_r nv_localtime_r
#define gmtime_r nv_gmtime_r

static inline int nv_setenv(const char *name, const char *value, int overwrite) {
  if (!overwrite && getenv(name)) return 0;
  return _putenv_s(name, value ? value : "");
}
#define setenv nv_setenv

static inline int nv_fsync(int fd) { return _commit(fd); }
#define fsync nv_fsync

static inline int nv_fchmod(int fd, int mode) { (void)fd; (void)mode; return 0; }
#define fchmod nv_fchmod

static inline int nv_lstat(const char *path, struct stat *st) { return stat(path, st); }
#define lstat nv_lstat
/* Approximation sufficient for desktop cache accounting during W0. */
#define st_blocks st_size
#ifndef S_ISLNK
#define S_ISLNK(mode) 0
#endif

static inline ssize_t nv_readlink(const char *path, char *buf, size_t size) {
  (void)path; (void)buf; (void)size; errno = ENOSYS; return -1;
}
#define readlink nv_readlink

static inline char *nv_realpath(const char *path, char *resolved) {
  return _fullpath(resolved, path, PATH_MAX);
}
#define realpath nv_realpath

static inline unsigned nv_rand_r(unsigned *seed) {
  *seed = *seed * 1103515245u + 12345u;
  return (*seed / 65536u) % 32768u;
}
#define rand_r nv_rand_r

static inline time_t nv_timegm(struct tm *t) { return _mkgmtime(t); }
#define timegm nv_timegm

static inline FILE *nv_fmemopen(void *buf, size_t size, const char *mode) {
  FILE *f;
  (void)mode;
  f = tmpfile();
  if (!f) return NULL;
  if (size && fwrite(buf, 1, size, f) != size) { fclose(f); return NULL; }
  rewind(f);
  return f;
}
#define fmemopen nv_fmemopen

#ifndef ATOMIC_VAR_INIT
#define ATOMIC_VAR_INIT(value) (value)
#endif

#ifndef strtok_r
#define strtok_r strtok_s
#endif

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

static inline char *nv_strcasestr(const char *haystack, const char *needle) {
  size_t n;
  const char *p;
  if (!haystack || !needle) return NULL;
  n = strlen(needle);
  if (!n) return (char *)haystack;
  for (p = haystack; *p; p++)
    if (_strnicmp(p, needle, n) == 0) return (char *)p;
  return NULL;
}
#define strcasestr nv_strcasestr

/* POSIX mkdir(path, mode) -> Win32 _mkdir(path). */
#define mkdir(path, mode) _mkdir(path)

/* W0 does not need Unix advisory locks; the app already has an in-process mutex. */
#ifndef LOCK_EX
#define LOCK_EX 2
#define LOCK_UN 8
#endif
static inline int nv_flock(int fd, int op) { (void)fd; (void)op; return 0; }
#define flock nv_flock

#endif
#endif
