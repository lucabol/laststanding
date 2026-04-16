/* compat/math.h — Float-suffixed math shims for freestanding builds.
   l_os.h provides double-precision math via the standard names.
   NanoSVG needs float-suffixed versions; these wrappers cast through the
   existing double-precision entry points. */
#ifndef L_COMPAT_MATH_H
#define L_COMPAT_MATH_H

#ifndef sinf
static inline float sinf(float x)   { return (float)sin((double)x); }
#endif
#ifndef cosf
static inline float cosf(float x)   { return (float)cos((double)x); }
#endif
#ifndef tanf
static inline float tanf(float x)   { return (float)tan((double)x); }
#endif
#ifndef sqrtf
static inline float sqrtf(float x)  { return (float)sqrt((double)x); }
#endif
#ifndef fabsf
static inline float fabsf(float x)  { return (float)fabs((double)x); }
#endif
#ifndef ceilf
static inline float ceilf(float x)  { return (float)ceil((double)x); }
#endif
#ifndef floorf
static inline float floorf(float x) { return (float)floor((double)x); }
#endif
#ifndef fmodf
static inline float fmodf(float x, float y) { return (float)fmod((double)x, (double)y); }
#endif
#ifndef acosf
static inline float acosf(float x)  { return (float)acos((double)x); }
#endif
#ifndef atan2f
static inline float atan2f(float y, float x) { return (float)atan2((double)y, (double)x); }
#endif
#ifndef powf
static inline float powf(float b, float e) { return (float)pow((double)b, (double)e); }
#endif
#ifndef roundf
static inline float roundf(float x) { return (float)round((double)x); }
#endif
#ifndef isnan
#define isnan(x) ((x) != (x))
#endif

#endif /* L_COMPAT_MATH_H */
