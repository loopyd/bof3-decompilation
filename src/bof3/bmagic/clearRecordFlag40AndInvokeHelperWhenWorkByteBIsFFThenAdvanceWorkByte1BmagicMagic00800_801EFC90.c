#include "bof3/bof3.h"

/* Shared slot-4 event dispatcher of the battle overlays at 0x800AEF98, outside
 * this overlay's image and owned by the battle targets (raw entry,
 * emi/battle/battle/15@0x800AEF98). This lift only calls it, so the prototype
 * follows the call site: the first argument is a byte-sized event selector and
 * the second a byte-sized value that the dispatcher masks, and no result is
 * used. Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, where the overlay's asm spans reference it, so no
 * target-map row is added here. */
void func_800AEF98(s32 event, s32 value);

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers.
 * The original re-materializes the cell address (lui $vN,0x1F80 / lw
 * $vN,68($vN)) for each of the two reads in this function, so the cell is
 * declared volatile like the exact sibling
 * decrementMotionCountdownAndSubtractMotionPairThenAdvanceWorkByte2
 * BmagicMagic00800_801F04DC. */
extern u8 *volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The sibling bmagic lifts (clearRecordFlag40AndInvokeHelperWhenWorkByteBIsFF
 * ThenAdvanceWorkByte1BmagicMagic00800_801EF9D4,
 * seedMotionTintAndChannelFieldsThenAdvanceWorkByte2BmagicMagic00800_801F05FC)
 * read the same record's byte 0xB; splat binds the address through this
 * target's generated undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EFC90
 * @behavior Work byte 0x1 state 1 of this overlay: entry 1 of the three-entry
 *           handler table at 0x801EEC40 (0x801EFAD4, 0x801EFC90, 0x801EFCFC),
 *           which 0x801EFA74 dispatches on the scratchpad work object's state
 *           byte 0x1. It reads the work-object pointer cell at 0x1F800044 and,
 *           only when the work object's byte at offset 0xB is 0xFF, clears bit
 *           0x40 of byte 0 of the shared battle work record at 0x801EB4E0
 *           (through the 0xBF mask), invokes the shared battle helper at
 *           0x800AEF98 with the event selector 4 and the value 0, and then
 *           re-reads the scratchpad cell and increments work byte 0x1 with a
 *           byte-width add, which selects table entry 2 (0x801EFCFC). The
 *           record base stays in one register across its read, mask and store,
 *           and the original schedules that store into the helper call's delay
 *           slot. The 0x18-byte frame only keeps $ra across that call.
 *           Nothing is returned and no other state is read or written. Its
 *           instruction sequence is byte-identical to the sibling entry at
 *           0x801EF9D4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordFlag40AndInvokeHelperWhenWorkByteBIsFFThenAdvanceWorkByte1BmagicMagic00800_801EFC90(
    void) {
  if (g_battle_work[0xB] == 0xFF) {
    D_801EB4E0[0] &= 0xBF;
    func_800AEF98(4, 0);
    g_battle_work[1]++;
  }
}
