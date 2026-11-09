#include "bof3/bof3.h"

/* Shared slot-4 event dispatcher of the battle overlays at 0x800AEF98, outside
 * this overlay's image and owned by the battle targets (raw entry,
 * emi/battle/battle/15@0x800AEF98). This lift only calls it, so the prototype
 * follows the call site: the first argument is a byte-sized event selector and
 * the second a byte-sized value that the dispatcher masks, and no result is
 * used. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is added here. */
void func_800AEF98(s32 event, s32 value);

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers.
 * The original re-materializes the cell address (lui $vN,0x1F80 / lw
 * $vN,68($vN)) for both reads in this function, so the cell is declared as the
 * named fixed address the exact siblings
 * addWorkColorTripletBy8ThenDecrementWorkByte9ThenDecrementGlobalWorkByteB
 * AndAdvanceWorkByte2OnZeroBmagicMagic01703_801EF8B4 and
 * setWorkByte9To8AndAdvanceWorkByte2WhenHelperReadyBmagicMagic01703_801EF694
 * use. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The sibling bmagic lifts (clearRecordFlag40AndInvokeHelperWhenWorkByteBIsFF
 * ThenAdvanceWorkByte1BmagicMagic00800_801EF9D4 and ..._801EFC90) read the same
 * record's byte 0 through the 0xBF mask; splat binds the address through this
 * target's generated undefined_syms_auto.txt, so no target-map row is added
 * here. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EEF68
 * @behavior Work byte 1 state 1 of this overlay: entry 1 of the three-word
 *           handler table at 0x801EEC04 (0x801EEC98, 0x801EEF68, 0x801EEFD8),
 *           which func_801EEC38 copies into its frame and indexes with the
 *           scratchpad work object's byte 1, so the body runs while work byte 1
 *           is 1. It reads the work-object pointer cell at 0x1F800044 and, only
 *           when the work object's byte at offset 0xB is zero, invokes the
 *           shared battle helper at 0x800AEF98 with the event selector 4 and the
 *           value 0, then clears bit 0x40 of byte 0 of the shared battle work
 *           record at 0x801EB4E0 through the 0xBF mask, and finally re-reads the
 *           scratchpad cell and increments work byte 1 with a byte-width add,
 *           which selects table entry 2 (0x801EEFD8). The 4 selector is
 *           materialized ahead of the guard branch in its delay slot, the
 *           0x18-byte frame only keeps $ra at 0x10 across that call, and the
 *           pointer-cell load precedes frame creation as in the sibling
 *           dispatchWorkByte1HandlerBmagicMagic01703_801EF014. It takes no
 *           arguments, returns nothing and reads or writes no other state. The
 *           body matches invokeHelperThenClearRecordFlag40WhenWorkByteBIsFF
 *           ThenAdvanceWorkByte1BmagicMagic00800_801EEF6C in its statement order
 *           and operand forms, differing only in the byte-0xB test value, which
 *           is zero here and 0xFF there.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperThenClearRecordFlag40WhenWorkByteBZeroThenAdvanceWorkByte1BmagicMagic01703_801EEF68(
    void) {
  if (D_1F800044[0xB] == 0) {
    func_800AEF98(4, 0);
    D_801EB4E0[0] &= 0xBF;
    D_1F800044[1]++;
  }
}
