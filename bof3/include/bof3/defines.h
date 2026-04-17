#ifndef BOF3_DEFINES_H
#define BOF3_DEFINES_H

#include <stddef.h>

#ifndef __cplusplus
typedef unsigned char bool;
#define true  1
#define false 0
#endif

#define ARRAY_COUNT(values) (sizeof(values) / sizeof((values)[0]))

/*
 * Generic decomp scalar aliases.
 *
 * Keep this surface small and reusable: plain fixed-width scalars, floating
 * types, and their volatile variants.
 */

/* clang-format off */
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;
typedef float              f32;
typedef double             f64;
typedef volatile u8        vu8;
typedef volatile u16       vu16;
typedef volatile u32       vu32;
typedef volatile s8        vs8;
typedef volatile s16       vs16;
typedef volatile s32       vs32;
/* clang-format on */

#endif
