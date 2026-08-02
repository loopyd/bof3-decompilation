#ifndef EMI_SHOP_00_INTERNAL_H
#define EMI_SHOP_00_INTERNAL_H

#include "bof3/context.h"
#include "panel/task.h"
#include "gpu/prim.h"

extern PanelTask* D_80148648;
#define D_80148648 g_PanelTaskRoot

/* Absolute-address globals (byte-width counters/flags). */
extern volatile u8  D_80148650;
extern volatile u8  D_80148651;
extern volatile u8  D_80148652;
extern volatile u8  D_80148654;
extern volatile u16 D_801490A4;

/* Shared primitive cursor (PsyQ SDK, owned by the main exe). */
extern u8* D_8014598C;
#define D_8014598C g_PrimCursor

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * func_8014E5A0 is a game primitive-append helper (lifted in exe/slus_004_22). */
void func_8014E5A0(u32 ot_index, u32 primitive_size);

void func_801647C4(u16 arg0, u16 arg1, s32 arg2);

/* Shop command jump table (this EMI, text blob T_801E5144). */
extern void (*D_801E5D2C[])(u32);

void func_801E2CDC(void);
void func_801E3EF4(void);
void func_801E31C4(void);
void func_801E3774(void);
void func_801E3BF8(void);
void func_801E3D4C(void);
void func_801E438C(void);
void func_801E4540(void);
void func_801E3CB8(void);

#endif
