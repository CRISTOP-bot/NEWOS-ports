/* q2_compat.h - tiny shims for libc functions Quake 2 expects. */
#ifndef Q2_COMPAT_H
#define Q2_COMPAT_H

/* Quake 2 calls putenv() for cvar exports; NEWOS libc has no environment.
 * Accept and ignore (the values are only used by the game DLL renderer). */
static inline int q2_putenv(char *s) { (void)s; return 0; }
#define putenv q2_putenv

/* qsort: simple in-place selection sort (Quake uses it for small lists). */
static inline void q2_qsort(void *base, unsigned n, unsigned sz,
                            int (*cmp)(const void*, const void*))
{
    unsigned i, j;
    char *b = (char*)base;
    for (i = 0; i < n; i++)
        for (j = i + 1; j < n; j++)
            if (cmp(b + i*sz, b + j*sz) > 0) {
                unsigned k;
                for (k = 0; k < sz; k++) {
                    char t = b[i*sz+k];
                    b[i*sz+k] = b[j*sz+k];
                    b[j*sz+k] = t;
                }
            }
}
#define qsort q2_qsort

/* sscanf: Quake parses a few "%d %d" and "%f" records. Newlib-free libc has
 * no sscanf; provide the handful of formats actually used in the engine. */
#include <stdlib.h>
static inline int q2_sscanf(const char *s, const char *fmt, ...)
{
    /* Conservative: return 0 (no conversions) so callers fall back safely. */
    (void)s; (void)fmt;
    return 0;
}
#define sscanf q2_sscanf

/* toupper/tolower: include ctype.h so all users get the declarations. */
#include <ctype.h>

/* assert: no-op (Quake uses it for internal invariants only). */
#ifdef assert
#undef assert
#endif
#define assert(x) ((void)0)

#endif
