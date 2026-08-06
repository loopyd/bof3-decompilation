#ifndef EMI_SISYOU_00_INTERNAL_H
#define EMI_SISYOU_00_INTERNAL_H

#include "bof3/context.h"
#include "gpu/prim.h"

/* @source 0x8014598C @kind unknown */
/* Shared primitive cursor (PsyQ SDK, owned by the main exe). */
extern u8* D_8014598C;
#define D_8014598C g_PrimCursor

/* Main-RAM globals owned by the loaded image. */
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x801448ED @kind bss */
/* Index of the active master; selects the entry whose action base is read
 * from masterActionBaseTable. */
extern u8 masterIndex;

/* EMI-local data. */
/* @source 0x801D41BC @kind table */
/* Per-master u16 action-id base; adding 4 or 0x11 yields the action id
 * dispatched for the selected master. */
extern u16 masterActionBaseTable[];
/* @source 0x801D41E0 @kind table */
/* Per-mode handler table; indexed by modeIndex and tail-called. */
extern void (*D_801D41E0[])(void);
/* @source 0x801D41FC @kind table */
/* Handler table indexed by D_801D4286 and tail-called. */
extern void (*D_801D41FC[])(void);
/* @source 0x801D4204 @kind table */
/* Handler table indexed by D_801D4286 and tail-called. */
extern void (*D_801D4204[])(void);
/* @source 0x801D421C @kind table */
/* Handler table indexed by D_801D4286 and tail-called. */
extern void (*D_801D421C[])(void);
/* @source 0x801D4285 @kind bss */
/* Current mode index; selects the handler from the D_801D41E0 table and is
 * set to 6 after an entry action starts. */
extern u8 modeIndex;
/* @source 0x801D4286 @kind unknown */
/* Index selecting the handler from the D_801D41FC table. */
extern u8 D_801D4286;

/* PsyQ SDK primitive setup helpers called by this target.
 * SetSprt8 / SetSemiTrans are declared by <libgpu.h> (via bof3/psyq.h);
 * func_8014E5A0 is a game primitive-append helper (lifted in exe/slus_004_22). */
void func_8014E5A0(u32 ot_index, u32 primitive_size);

/* Main-exe functions called by this overlay. */
void func_80150224(s32 arg0);

/* EMI-local functions. */
void func_801D10AC(u32 arg0);
void func_801D25D8(void);
void func_801D2BE8(void);

#endif
