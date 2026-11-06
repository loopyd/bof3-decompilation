#include "bof3/bof3.h"

/* Shared slot-4 event dispatcher of the battle overlays at 0x800AEF98, outside
 * this overlay's image and owned by the battle targets (raw entry,
 * emi/battle/battle/15@0x800AEF98). This lift only calls it, so the prototype
 * follows the call site: the first argument is a byte-sized event selector and
 * the second a byte-sized value that the dispatcher masks, and no result is
 * used. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, where the overlay's remaining asm spans reference
 * it, so no target-map row is added here. */
void func_800AEF98(s32 event, s32 value);

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers.
 * The original re-materializes the cell address (lui $vN,0x1F80 / lw
 * $vN,68($vN)) for each of the two reads in this function, so the cell is
 * declared volatile like the exact siblings
 * clearRecordFlag40AndInvokeHelperWhenWorkByteBIsFFThenAdvanceWorkByte1
 * BmagicMagic00800_801EF9D4 and ..._801EFC90. */
extern u8 *volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The sibling bmagic lifts (clearRecordFlag40AndInvokeHelperWhenWorkByteBIsFF
 * ThenAdvanceWorkByte1BmagicMagic00800_801EF9D4 and ..._801EFC90,
 * decrementMotionCountdownAndSubtractMotionPairThenAdvanceWorkByte2
 * BmagicMagic00800_801F04DC) read the same record's byte 0; splat binds the
 * address through this target's generated undefined_syms_auto.txt, so no
 * target-map row is added here. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EEF6C
 * @behavior Work byte 1 state 1 of this overlay: entry 1 of the four-entry
 *           handler table at 0x801EEC04 (0x801EED5C, 0x801EEF6C, 0x801EEFDC,
 *           0x801EF010), which 0x801EECF0 dispatches on the scratchpad work
 *           object's state byte 1. It reads the work-object pointer cell at
 *           0x1F800044 and, only when the work object's byte at offset 0xB is
 *           0xFF, invokes the shared battle helper at 0x800AEF98 with the event
 *           selector 4 and the value 0, then clears bit 0x40 of byte 0 of the
 *           shared battle work record at 0x801EB4E0 through the 0xBF mask, and
 *           finally re-reads the scratchpad cell and increments work byte 1
 *           with a byte-width add, which selects table entry 2 (0x801EEFDC).
 *           The 0x18-byte frame only keeps $ra across that call. Nothing is
 *           returned and no other state is read or written. Unlike the sibling
 *           entries at 0x801EF9D4 and 0x801EFC90, which clear the record before
 *           the call, this entry calls first and clears the record afterwards.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperThenClearRecordFlag40WhenWorkByteBIsFFThenAdvanceWorkByte1BmagicMagic00800_801EEF6C(
    void) {
  if (g_battle_work[0xB] == 0xFF) {
    func_800AEF98(4, 0);
    D_801EB4E0[0] &= 0xBF;
    g_battle_work[1]++;
  }
}
