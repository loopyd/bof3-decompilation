#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include <stddef.h>

#ifndef __cplusplus
typedef unsigned char bool;

#define false 0
#define true  1
#endif

/* clang-format off */
typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef float              f32;
typedef double             f64;
/* clang-format on */

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

/*
 * C89-compatible compile-time assertions.
 */
#define DECOMP_JOIN_IMPL(a, b) a##b
#define DECOMP_JOIN(a, b)      DECOMP_JOIN_IMPL(a, b)

#define STATIC_ASSERT(condition)                                               \
  typedef char DECOMP_JOIN(static_assertion_, __LINE__)[(condition) ? 1 : -1]

#define ASSERT_SIZE(type, expected_size)                                       \
  STATIC_ASSERT(sizeof(type) == (expected_size))

#define ASSERT_OFFSET(type, member, expected_offset)                           \
  STATIC_ASSERT(offsetof(type, member) == (expected_offset))

#endif
