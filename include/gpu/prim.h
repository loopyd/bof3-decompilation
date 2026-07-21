#ifndef GPU_PRIM_H
#define GPU_PRIM_H

#include "base/types.h"
#include "memory/access.h"

#define g_PrimCursor PSX_REF(u8*, 0x8014598Cu)

#define g_UiCharBuffer PSX_PTR(volatile u8, 0x80145AD4u)

#define GpuAppendPrim func_8014E5A0

void GpuAppendPrim(u32 ot_index, u32 primitive_size);
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

#endif
