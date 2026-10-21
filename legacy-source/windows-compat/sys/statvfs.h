#ifndef STREAMMAX_WINDOWS_STATVFS_H
#define STREAMMAX_WINDOWS_STATVFS_H
#include <windows.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
struct statvfs {
  unsigned long f_bsize, f_frsize;
  uint64_t f_blocks, f_bfree, f_bavail;
  uint64_t f_files, f_ffree, f_favail;
  unsigned long f_fsid, f_flag, f_namemax;
};
static inline int statvfs(const char *path, struct statvfs *s) {
  ULARGE_INTEGER avail, total, freeb;
  char root[MAX_PATH] = {0};
  if (!s) return -1;
  memset(s, 0, sizeof *s);
  if (!path || !*path) path = ".";
  if (!GetFullPathNameA(path, MAX_PATH, root, NULL)) return -1;
  if (root[1] == ':') { root[2] = '\\'; root[3] = 0; }
  else strcpy(root, ".\\");
  if (!GetDiskFreeSpaceExA(root, &avail, &total, &freeb)) return -1;
  s->f_bsize = s->f_frsize = 4096;
  s->f_blocks = total.QuadPart / 4096;
  s->f_bfree = freeb.QuadPart / 4096;
  s->f_bavail = avail.QuadPart / 4096;
  s->f_namemax = 255;
  return 0;
}
#endif
