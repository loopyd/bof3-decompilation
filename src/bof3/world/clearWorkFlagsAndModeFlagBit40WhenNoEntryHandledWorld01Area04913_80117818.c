#include "bof3/bof3.h"

/* Record pass of this overlay, still on its asm row: republishes the 0x800E4800
 * work table record by record in world01Area04913WorkRecordCursor, advances and
 * renders every claimed record of it, and leaves 1 in its return register once
 * it advanced at least one claimed record and 0 when none was claimed. */
s32 func_80117900(void);

/* Shared work-area reset clearWorkFlags in emi/etc/game/00: zeroes work-object
 * bytes 0x00-0x04 and so returns the overlay to work state 0. */
void func_80196070(void);

/* World/mode flag halfword of the shared main executable, at the same fixed
 * address area01613 reads. This body clears its bit 0x40, the bit the state-4
 * handler of this overlay raises; no lift of that executable names the bit yet,
 * so the halfword and the bit stay address-anchored.
 * @source 0x80146258 @kind unknown
 */
extern u16 D_80146258;

/* @source 0x80117818
 * @behavior State-7 handler of the thirteen in-image pointers the Splat textbin
 * row D_8011818C places: word 7 at 0x801181A8, the base
 * callRunRecordPass indexes
 * from the work object byte at offset 1 once that byte reads 7, the state the
 * state-6 handler lowerMirrorEveryFourth advances to. It runs the overlay
 * record pass func_80117900, which walks the sixteen records of 0x28-byte stride
 * in the 0x800E4800 work table, advancing and rendering each claimed one, and
 * leaves 1 in its return register once it advanced at least one claimed record
 * and 0 when none was claimed. When that result masked to its low byte is zero
 * it resets the work object through the shared func_80196070 (clearWorkFlags in
 * emi/etc/game/00), which zeroes work-object bytes 0x00-0x04 and so returns the
 * overlay to work state 0, and then clears bit 0x40 of the shared world/mode
 * flag halfword 0x80146258, the bit the state-4 handler
 * raiseFlagReseedRecords
 * raised. Takes no arguments and returns nothing; the 0x18-byte frame only keeps
 * $ra at 0x10 across the two calls, the reset and the flag clear are reached
 * only on the zero path, and the slots after both calls stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlagsAndModeFlagBit40WhenNoEntryHandledWorld01Area04913_80117818(void) {
  if ((func_80117900() & 0xFF) == 0) {
    func_80196070();
    D_80146258 &= 0xFFBF;
  }
}
