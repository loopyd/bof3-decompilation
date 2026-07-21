#ifndef GPU_PRIM_H
#define GPU_PRIM_H

#include "base/types.h"
#include "memory/access.h"

#define g_PrimCursor PSX_REF(u8*, 0x8014598Cu)

#define g_UiCharBuffer PSX_PTR(volatile u8, 0x80145AD4u)

#define GpuAppendPrim func_8014E5A0

#endif
