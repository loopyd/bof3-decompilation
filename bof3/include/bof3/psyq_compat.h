#ifndef BOF3_PSYQ_COMPAT_H
#define BOF3_PSYQ_COMPAT_H

#include "bof3/defines.h"

#ifndef _UCHAR_T
#define _UCHAR_T
typedef unsigned char u_char;
#endif

#ifndef _USHORT_T
#define _USHORT_T
typedef unsigned short u_short;
#endif

#ifndef _UINT_T
#define _UINT_T
typedef unsigned int u_int;
#endif

#ifndef _ULONG_T
#define _ULONG_T
typedef unsigned long u_long;
#endif

/* clang-format off */
#include <libgte.h>
#include <libgpu.h>
/* clang-format on */

#endif
