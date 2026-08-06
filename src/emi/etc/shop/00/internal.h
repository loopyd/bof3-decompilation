#ifndef EMI_SHOP_00_INTERNAL_H
#define EMI_SHOP_00_INTERNAL_H

#include "bof3/context.h"
#include "panel/task.h"
#include "gpu/prim.h"

/* @source 0x80148648
 * @kind unknown */
extern PanelTask* D_80148648;
#define D_80148648 g_PanelTaskRoot

/* Absolute-address globals (byte-width counters/flags). */
/* Write-only in this target (7 stores, no loads); role unproven.
 * @source 0x80148650
 * @kind unknown */
extern volatile u8  D_80148650;
/* UI phase byte: reset with the timer on phase changes and stepped (+1/+2)
 * by the phase handlers. Kept raw: the shared map owns this address and
 * emi/etc/game/00 consumes it as D_80148651.
 * @source 0x80148651
 * @kind unknown */
extern volatile u8  D_80148651;
/* Sub-step counter: advanced when phaseTimer wraps and by the step
 * handler, cleared by the UI-state reset. Kept raw (shared-map address).
 * @source 0x80148652
 * @kind unknown */
extern volatile u8  D_80148652;
/* @source 0x80148654
 * @kind bss — per-frame phase timer; decremented each tick, zeroed on phase
 * changes, advances D_80148652 on wrap. */
extern volatile u8  phaseTimer;
/* @source 0x801490A4
 * @kind unknown */
extern volatile u16 D_801490A4;

/* Shared primitive cursor (PsyQ SDK, owned by the main exe).
 * @source 0x8014598C
 * @kind unknown */
extern u8* D_8014598C;
#define D_8014598C g_PrimCursor

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * func_8014E5A0 is a game primitive-append helper (lifted in exe/slus_004_22). */
void func_8014E5A0(u32 ot_index, u32 primitive_size);

void func_801647C4(u16 arg0, u16 arg1, s32 arg2);

/* @source 0x801E5D2C
 * @kind table — shop command handler pointers (this EMI, text blob
 * T_801E5144); dispatched by dispatchCommand with the command id
 * scaled by 4 as the handler argument. */
extern void (*commandHandlerTable[])(u32);

/* @source 0x801E52C4
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D39BC with the phase byte D_80148651 as index. */
extern void (*D_801E52C4[])(void);

/* @source 0x801E5360
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D6184 with the phase byte D_80148651 as index. */
extern void (*D_801E5360[])(void);

/* @source 0x801E52D8
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D3AA8 with the phase byte D_80148652 as index. */
extern void (*D_801E52D8[])(void);

/* @source 0x801E52F0
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D41B0 with the phase byte D_80148652 as index. */
extern void (*D_801E52F0[])(void);

/* @source 0x801E530C
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D46A4 with the phase byte D_80148652 as index. */
extern void (*D_801E530C[])(void);

#endif
