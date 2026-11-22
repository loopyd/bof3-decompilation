#include "bof3/bof3.h"

void func_800A9610(u8 arg0, s32 arg1);

/* Scratchpad work-object pointer cell, shared by this overlay's step handlers.
 * The original re-materialises its address at each access (lui $vN,0x1F80 /
 * lw $vN,0x44($vN)) because a store through the loaded pointer may alias the
 * cell, so it stays a named cell: all three reads of this function load the cell
 * again. The row lives in this target's map, the same one the exact siblings
 * 0x801EF374 and 0x801EF3C4 use. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle target byte at 0x80146384. It lives outside this overlay's
 * image and this target's generated undefined_syms_auto.txt already binds the
 * address, so the name needs no map row. The exact bmagic siblings
 * raiseBattleTargetFlag10WhenWorkByte9SetThenAdvanceWorkByte1BmagicMagic10003_801EED9C
 * and raiseBattleTargetFlag20ClearBytes9AThenAdvanceWorkByte1BmagicMagic09403_801EEF1C
 * forward the same cell to the same helper, and the byte-identical exact sibling
 * decrementWorkByteBIncrementByte9RaiseFlag10At18ThenAdvanceWorkByte2BmagicMagic12203_801EF1C4
 * reads it the same way. Its role stays address-anchored here. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* @source 0x801EF2EC
 * @behavior Work-byte-2 step handler of the overlay work object: it is entry 2
 *           of the code pointer table based at 0x801F0CB0, the word at
 *           0x801F0CB8 of this payload, and no jal word names it. The dispatcher
 *           dispatchWorkByte2HandlerThenCallF44CWhenGlobalWorkByteBNotFFAndWorkByte2SetBmagicMagic12003_801EF054
 *           reaches it by scaling the work object's byte 2 by four and calling
 *           that entry, so this body runs when that byte holds 2. It decrements
 *           the work object's byte 0xB by one through the scratchpad pointer cell
 *           0x1F800044 (lbu / addiu -1 / sb; the stored result is not used, so no
 *           truncation mask appears), then increments the object's byte 9 and
 *           tests the truncated value it just stored (andi 0xFF of the stored
 *           register, no second load of the byte). Only when that counter reached
 *           0x18 does it read the shared battle target byte at 0x80146384 as an
 *           unsigned byte and pass it with the constant 0x10 to func_800A9610
 *           before re-reading the pointer cell and incrementing the object's byte
 *           2, which advances the work step. The 0x18-byte frame only keeps $ra
 *           across that single call, and that call's delay slot carries the 0x10
 *           constant. The leading cell group loads the pointer before the frame
 *           is created because the decrement needs no stack. It takes no
 *           arguments, returns nothing through the jr $ra at 0x801EF36C and
 *           reads or writes no other state; the roles of work-object bytes 0xB,
 *           9 and 2 and of the flag word 0x10 stay offset-anchored.
 *
 *           Statement order, operand forms, offset spelling and declaration
 *           style are that of the byte-identical exact sibling
 *           emi/bmagic/magic122/03 0x801EF1C4, one payload comparison over the
 *           136 bytes of both spans. Two exact siblings of this same target back
 *           the same regions: 0x801EF374 carries the truncated byte-9 test and
 *           the byte-2 advance, and 0x801EF3C4 carries the leading cell group,
 *           the frame and the tail.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteBIncrementByte9RaiseFlag10At18ThenAdvanceWorkByte2BmagicMagic12003_801EF2EC(void) {
  g_battle_work[0xB] -= 1;
  if ((g_battle_work[9] += 1) == 0x18) {
    func_800A9610(D_80146384, 0x10);
    g_battle_work[2] += 1;
  }
}
