#include "bof3/bof3.h"

void func_800A9610(u8 arg0, s32 arg1);

/* Scratchpad work-object pointer cell. The original re-materialises its address
 * at each access (lui $vN,0x1F80 / lw $vN,0x44($vN)) because a store through the
 * loaded pointer may alias the cell, so it stays a named cell: all three reads of
 * this function load the cell again. Bound at 0x1F800044 by this target's map,
 * the same row the exact sibling
 * incrementWorkByte9AndAdvanceWorkByte2At78BmagicMagic12203_801EF24C uses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle target byte at 0x80146384: the target selector the battle
 * helpers consume (its 0x80/0x40 bits pick the party or enemy group). It lives
 * outside this overlay, and the overlay's remaining asm still references it, so
 * Splat's generated undefined_syms_auto.txt binds it and no target-map row is
 * needed. The exact bmagic siblings
 * raiseBattleTargetFlag10WhenWorkByte9SetThenAdvanceWorkByte1BmagicMagic10003_801EED9C
 * and raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09403_801EEF1C
 * forward the same cell to the same helper. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EF1C4
 * @behavior Frame step of the overlay work object: entry 3 of the six-entry
 *           handler table at 0x801F02CC that this overlay's byte-1 dispatcher
 *           dispatchStateByte1Table02ccBmagicMagic12203_801EEE5C indexes with
 *           work byte 1. It decrements the work object's byte 0xB by one through
 *           the scratchpad pointer cell 0x1F800044 (lbu / addiu -1 / sb; the
 *           stored result is not used, so no truncation mask appears), then
 *           increments the work object's byte 9 and tests the truncated value it
 *           just stored (andi 0xFF of the stored register, no second load of the
 *           byte). Only when that counter reached 0x18 does it read the shared
 *           battle target byte at 0x80146384 (unsigned byte load, zero-extended)
 *           and pass it with the flag word 0x10 to func_800A9610, which raises
 *           that battler status flag, before re-reading the pointer cell and
 *           incrementing the work object's byte 0x2 to advance the work step.
 *           The 0x18-byte frame only keeps $ra across that single call, the
 *           constant 0x10 fills the call's delay slot, and the cell is read once
 *           per store group because each store through the loaded pointer may
 *           alias it. It takes no arguments, returns nothing and reads or writes
 *           no other state. The byte-9 increment, the truncating test it owns and
 *           the byte-0x2 advance follow the exact sibling of this same overlay,
 *           which differs only in its 0x78 threshold and in having no flag call
 *           and no byte-0xB countdown; the flag-0x10 call through the named
 *           target byte follows the two exact bmagic siblings named above. The
 *           same 136 bytes appear again, unlifted, as func_801EF2EC of
 *           emi/bmagic/magic120/03, so this handler shape is repeated across the
 *           bmagic overlays, but no authored source of those bytes exists to
 *           clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteBIncrementByte9RaiseFlag10At18ThenAdvanceWorkByte2BmagicMagic12203_801EF1C4(void) {
  g_battle_work[0xB] -= 1;
  if ((g_battle_work[9] += 1) == 0x18) {
    func_800A9610(D_80146384, 0x10);
    g_battle_work[2] += 1;
  }
}
