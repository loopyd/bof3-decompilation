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

/*
 * PSX hardware I/O register accessors.
 *
 * For 0x1f80xxxx memory-mapped registers (GPU, GTE, CD, SPU, etc.).
 *
 * Usage: REG32(0x1f801080) |= 1;  REG16(0x1f8010f0) = 0x200;
 */
#define REG8(addr)  (*(volatile u8*)(addr))
#define REG16(addr) (*(volatile u16*)(addr))
#define REG32(addr) (*(volatile u32*)(addr))

/*
 * Volatile RAM accessors.
 *
 * Use REG8/REG16/REG32 for hardware I/O (0x1f80xxxx).
 * Use extern type DAT_xxxxx; + SYMBOL_AT(name, addr) for fixed-address
 * RAM globals (0x800xxxxx .. 0x801xxxxx).
 */
#define VPTR(type, addr)  ((volatile type*)(addr))
#define CVPTR(type, addr) ((const volatile type*)(addr))
#define VPPTR(type, addr) (*(volatile type**)(addr))

#endif
