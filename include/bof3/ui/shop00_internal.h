#ifndef EMI_SHOP_00_INTERNAL_H
#define EMI_SHOP_00_INTERNAL_H

#include "bof3/context.h"
#include "base/barrier.h"
#include "panel/task.h"
#include "gpu/prim.h"

/* @source 0x80148648
 * @kind unknown */
extern PanelTask* D_80148648;
#define D_80148648 g_PanelTaskRoot

/* @source 0x80143C40 @kind unknown */
extern volatile u16 D_80143C40;
extern u8 D_80148330[];
extern u8 D_80148331;
extern u8 D_80148332;
extern u8 D_80148333;
extern s16 D_80148334;
extern s16 D_80148336;
extern s16 D_80148340;
extern u8 D_80148355;
extern u8 D_80148356;
extern u8 D_80148357;
extern s16 D_80148358;
extern s16 D_8014835A;
extern u8 D_8014835C;
extern u8 D_8014835E;
extern u8 D_8014835F;
extern u8 D_80148360;
extern u8 D_80148361;
extern u8 D_80148378[];
extern u8 D_80148379;
extern u8 D_8014837A;
extern u8 D_8014837B;
extern s16 D_8014837C;
extern s16 D_8014837E;
extern u8 D_80148382;
extern u8 D_8014839D;
extern u8 D_8014839E;
extern u8 D_8014839F;
extern s16 D_801483A0;
extern s16 D_801483A2;
extern u8 D_80148625;
extern u8 D_80148626;

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
extern u8 D_80148652;
/* @source 0x8014865F @kind unknown */
extern volatile u8  D_8014865F;
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
extern u8* g_PrimCursor;

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * appendRenderPrim is a game primitive-append helper (lifted in exe/slus_004_22).
 * @source 0x8014E5A0 */
void appendRenderPrim(u32 ot_index, u32 primitive_size);

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

/* @source 0x801E545C
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D7344 with the phase byte D_80148651 as index. */
extern void (*D_801E545C[])(void);

/* @source 0x801E5BEC
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801DE188 with the phase byte D_80148651 as index. */
extern void (*D_801E5BEC[])(void);

/* @source 0x801E52D8
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D3AA8 with the phase byte D_80148652 as index. */
extern void (*D_801E52D8[])(void);

/* @source 0x801E5D10
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801E092C with the phase byte D_80148652 as index. */
extern void (*D_801E5D10[])(void);

/* @source 0x801E52F0
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D41B0 with the phase byte D_80148652 as index. */
extern void (*D_801E52F0[])(void);

/* @source 0x801E530C
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801D46A4 with the phase byte D_80148652 as index. */
extern void (*D_801E530C[])(void);

/* @source 0x801E5BFC
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801DE1C4 with the phase byte D_80148652 as index. */
extern void (*D_801E5BFC[])(void);

/* @source 0x801E5C08
 * @kind table */
extern void (*D_801E5C08[])(void);

/* @source 0x801E5CFC
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801DFDB8 with the phase byte D_80148652 as index. */
extern void (*D_801E5CFC[])(void);

/* @source 0x801E5CE8
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801DF978 with the phase byte D_80148651 as index. */
extern void (*D_801E5CE8[])(void);

/* @source 0x801E5D3C
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801E200C with the phase byte D_80148651 as index. */
extern void (*D_801E5D3C[])(void);

/* @source 0x801E5D48
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801E2048 with the sub-step byte D_80148652 as index. */
extern void (*D_801E5D48[])(void);

/* @source 0x801E5D50
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801E2114 with the sub-step byte D_80148652 as index. */
extern void (*D_801E5D50[])(void);

/* @source 0x801E5D5C
 * @kind table — UI phase handler pointers (this EMI); dispatched by
 * func_801E2590 with the sub-step byte D_80148652 as index. */
extern void (*D_801E5D5C[])(void);

/* @source 0x801E5D68
 * @kind table — handler pointers (this EMI); dispatched by func_801E27BC
 * with panel task byte 2 (g_PanelTaskRoot->unk_00[2]) as index. */
extern void (*D_801E5D68[])(void);

void appendFullscreenDimTileB(void);
void func_801DE8E8(void);

typedef struct ShopValueRecord {
  u16 value;
  u8 unk_02[6];
} ShopValueRecord;

extern ShopValueRecord D_801CB8DC[][99];

#endif
