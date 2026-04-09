/* compat/stdlib.h — Minimal shim for freestanding stb_image builds.
   stb_image only needs abs() from stdlib when STBI_MALLOC/FREE/REALLOC are defined
   and STBI_NO_STDIO is set. */
#ifndef L_COMPAT_STDLIB_H
#define L_COMPAT_STDLIB_H
#include <stddef.h>
#ifndef abs
static inline int abs(int x) { return x < 0 ? -x : x; }
#endif
#endif
