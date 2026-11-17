#ifndef GPU_PRIM_H
#define GPU_PRIM_H

#include "base/types.h"
#include "memory/access.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

#define g_UiCharBuffer PSX_PTR(volatile u8, 0x80145AD4u)

#define GpuAppendPrim func_8014E5A0

void GpuAppendPrim(u32 ot_index, u32 primitive_size);
/* SLUS text submitter stores the coordinate words as halfwords, with y + 1.
 * The attributes and run limit use their low bytes. */
void func_8014F800(s32 x, s32 y, s32 arg2, u32 arg3, u32 arg4);

#endif
