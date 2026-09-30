#ifndef EMI_WORLD00_AREA030_05_INTERNAL_H
#define EMI_WORLD00_AREA030_05_INTERNAL_H

#include "bof3/bof3.h"

extern u8 handlerIndex; /* @source 0x800F724C @kind data */
/* Phase byte advanced by the handler at handlerTable[3]; func_800F5048 reads it
 * and dispatches through the same table starting 0x0C bytes past its base. */
extern u8 handlerPhase; /* @source 0x800F724D @kind data */
extern void (*handlerTable[])(void); /* @source 0x800F71F0 @kind table */

/* Local frontend-entry bytes written by the setup entry at 0x800F50B4: an entry
 * flag cleared on entry (0x800F723C), the companion record index returned by
 * func_800F66D4 (0x800F7240), and a mode byte set to 1 (0x800F7244). */
extern u8 D_800F723C; /* @source 0x800F723C @kind unknown */
extern u8 D_800F7240; /* @source 0x800F7240 @kind unknown */
extern u8 D_800F7244; /* @source 0x800F7244 @kind unknown */

/* Shared gate halfword outside the AREA030/05 image, read by the phase-1
 * entry func_800F510C (lhu at 0x800F5110) and by the AREA030 companion target
 * as an AREA030 transition-idle flag. */
extern u16 D_80143C40; /* @source 0x80143C40 @kind unknown */

void dispatchArea030CompanionHandler(void); /* @source 0x800F500C */
void func_800F5048(void); /* @source 0x800F5048 */
void func_800F50B4(void); /* @source 0x800F50B4 */
void func_800F510C(void); /* @source 0x800F510C */

/* Companion record scan called by func_800F50B4 (jal at 0x800F50D8): walks the
 * 10-byte record at D_801E3134 + 10 * D_80143F1C and returns the first index
 * holding 0xFF, or 10 when the record is full. Boundary is still unreviewed
 * Splat assembly; the u8 result follows the callee's `andi $v0, $a1, 0xFF`. */
u8 func_800F66D4(void); /* @source 0x800F66D4 */

/* Local setup phases; boundaries are still unreviewed Splat assembly. */
void func_800F606C(s32 arg0);
void func_800F6730(void);
void func_800F5E44(void);

/* Companion-overlay call into BIN/WORLD00/AREA030.EMI#4 (0x801D9534, jal at
 * 0x800F5084); signature from the callee prologue (a0-a3 copied to s0-s3, first
 * three masked with 0xFFFF, fifth argument read from 0x40(sp)). A formal
 * companion record needs AREA030/04's target-local map to own 0x801D9534. */
s32 func_801D9534(s16 arg0, u16 arg1, s16 arg2, s16 arg3, s32 arg4);

#endif
