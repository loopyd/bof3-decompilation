#ifndef DEFINES_H
#define DEFINES_H

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
 * Use extern type D_xxxxx; + WEAK_SYMBOL_AT(name, addr) for fixed-address
 * RAM globals (0x800xxxxx .. 0x801xxxxx).
 */
#define VPTR(type, addr)  ((volatile type*)(addr))
#define CVPTR(type, addr) ((const volatile type*)(addr))

/* DEPRECATED: VPPTR uses (volatile type**) which makes only the target
 * object volatile, NOT the pointer cell. For scratchpad slots the cell
 * should be volatile too; use SPAD_PTR_SLOT / SPAD_VPTR_SLOT instead.
 * Kept for existing matched functions. */
#define VPPTR(type, addr) (*(volatile type**)(addr))

/*
 * PS1 scratchpad pointer cell at 0x1F800044 — holds a pointer to the
 * overlay's work area (different struct per overlay).
 *
 * See include/bof3/scratchpad.h for the preferred SPAD_* macros.
 *
 * For relocation-sensitive matching, each overlay declares a typed extern
 * (e.g. extern struct GameWorkArea* volatile g_game_work;) and adds
 * WEAK_SYMBOL_AT(g_game_work, 0x1F800044) in its symbols.c.
 */
#define GLOBAL_WORK (*(volatile u8**)0x80146250u)

/* Prevent sibling calls so the compiler emits exact jal instructions
 * matching the original binary's call graph. */
#if defined(__GNUC__)
#define NO_SIBLING_CALLS __attribute__((optimize("no-optimize-sibling-calls")))
#define COMPILER_BARRIER __asm__ __volatile__("" : : : "memory")
#else
#define NO_SIBLING_CALLS
#define COMPILER_BARRIER
#endif

#endif
