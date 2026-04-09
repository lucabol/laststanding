/* compat/string.h — Minimal shim for freestanding stb_image builds.
   l_os.h provides all needed string functions. This header is intentionally
   empty — l_img.h #defines memcpy/memcmp/etc. to l_os.h equivalents before
   including stb_image.h, so the declarations are not needed here. */
#ifndef L_COMPAT_STRING_H
#define L_COMPAT_STRING_H
/* Empty — all string functions are provided via l_os.h macros */
#endif
