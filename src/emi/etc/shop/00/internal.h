#ifndef EMI_SHOP_00_INTERNAL_H
#define EMI_SHOP_00_INTERNAL_H

#include "bof3/context.h"
#include "panel/task.h"
#include "gpu/prim.h"

extern PanelTask* D_80148648;
#define D_80148648 g_PanelTaskRoot

/* Absolute-address globals (byte-width counters/flags). */
/* Write-only in this target (7 stores, no loads); role unproven. */
extern volatile u8  D_80148650;
/* UI phase byte: reset with the timer on phase changes and stepped (+1/+2)
 * by the phase handlers. Kept raw: the shared map owns this address and
 * emi/etc/game/00 consumes it as D_80148651. */
extern volatile u8  D_80148651;
/* Sub-step counter: advanced when SHOP_PHASE_TIMER wraps and by the step
 * handler, cleared by the UI-state reset. Kept raw (shared-map address). */
extern volatile u8  D_80148652;
/* @kind: bss — per-frame phase timer; decremented each tick, zeroed on phase
 * changes, advances D_80148652 on wrap. */
extern volatile u8  SHOP_PHASE_TIMER;
extern volatile u16 D_801490A4;

/* Shared primitive cursor (PsyQ SDK, owned by the main exe). */
extern u8* D_8014598C;
#define D_8014598C g_PrimCursor

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * func_8014E5A0 is a game primitive-append helper (lifted in exe/slus_004_22). */
void func_8014E5A0(u32 ot_index, u32 primitive_size);

void func_801647C4(u16 arg0, u16 arg1, s32 arg2);

/* @kind: table — shop command handler pointers (this EMI, text blob
 * T_801E5144); dispatched by shop_dispatch_command with the command id
 * scaled by 4 as the handler argument. */
extern void (*shop_command_handlerTable[])(u32);

void shop_panel_x_advance_to_17(void);
void shop_panel_x_advance_to_17_2(void);
void shop_panel_x_advance_to_320(void);
void shop_panel_x_advance_to_320_2(void);
void shop_panel_x_advance_to_320_3(void);
void shop_panel_x_advance_to_320_4(void);
void shop_panel_x_advance_to_320_5(void);
void shop_panel_x_advance_to_320_6(void);
void shop_panel_x_retreat_to_neg170(void);

#endif
