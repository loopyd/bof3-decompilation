#ifndef EMI_SISYOU_00_INTERNAL_H
#define EMI_SISYOU_00_INTERNAL_H

#include "bof3/context.h"
#include "gpu/prim.h"

/* Shared primitive cursor (PsyQ SDK, owned by the main exe). */
extern u8* D_8014598C;
#define D_8014598C g_PrimCursor

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * func_8014E5A0 is a game primitive-append helper (lifted in exe/slus_004_22). */
void func_8014E5A0(u32 ot_index, u32 primitive_size);

#endif
