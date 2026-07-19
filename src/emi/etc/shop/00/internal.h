#ifndef EMI_SHOP_00_INTERNAL_H
#define EMI_SHOP_00_INTERNAL_H

#include "bof3/context.h"
#include "bof3/ui/panel_task.h"

extern Bof3PanelTask* D_80148648;

/* Absolute-address globals. */
extern volatile u32 D_80148650[];
extern volatile u32 D_80148651[];
extern volatile u32 D_80148652[];
extern volatile u32 D_801490A4[];

/* Shared primitive cursor (PsyQ SDK, owned by the main exe). */
extern u8* D_8014598C;

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * func_8014E5A0 is a game primitive-append helper (lifted in exe/slus_004_22). */
void func_8014E5A0(u32 ot_index, u32 primitive_size);

void func_801E2CDC(void);
void func_801E3EF4(void);
void func_801E31C4(void);
void func_801E3774(void);
void func_801E3BF8(void);
void func_801E3D4C(void);
void func_801E438C(void);
void func_801E4540(void);

#endif
