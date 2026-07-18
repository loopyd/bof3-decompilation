#ifndef BOF3_DEFINES_H
#define BOF3_DEFINES_H

#include <stddef.h>

#ifndef __cplusplus
typedef unsigned char bool;

#define true  1
#define false 0
#endif

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

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

/*
 * C89-compatible layout assertions.
 */

#define DECOMP_JOIN_IMPL(a, b) a##b
#define DECOMP_JOIN(a, b)      DECOMP_JOIN_IMPL(a, b)

#define STATIC_ASSERT(condition) \
  typedef char DECOMP_JOIN(static_assertion_, __LINE__)[(condition) ? 1 : -1]

#define ASSERT_SIZE(type, expected_size) \
  STATIC_ASSERT(sizeof(type) == (expected_size))

#define ASSERT_OFFSET(type, member, expected_offset) \
  STATIC_ASSERT(offsetof(type, member) == (expected_offset))

/*
 * Prevent sibling-call optimization when the original binary contains
 * a real call instruction.
 */
#if defined(__GNUC__)
#define NO_SIBLING_CALLS __attribute__((optimize("no-optimize-sibling-calls")))
#define barrier()        __asm__ __volatile__("" : : : "memory")
#else
#define NO_SIBLING_CALLS
#define barrier()
#endif

#endif
