#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This target's map binds it at 0x1F800044 as g_battle_work, the same binding
 * the exact siblings decrementWorkByteAAndAdvanceWorkByte2OnZeroBmagicMagic
 * 07803_801EF0E0, decrementWorkByteAThenSetGlobalWorkByteBTo1AndAdvanceWork
 * Byte2OnZeroBmagicMagic07803_801EF1CC and incrementWorkByte9Until8ThenInvoke
 * HelperAt101AndAdvanceWorkByte2BmagicMagic07803_801F00EC use. The original
 * loads the cell once before the 0x18-byte frame is created and
 * re-materializes it with its own lui/lw pair after the call, so the second
 * statement stays on the named cell instead of sharing the loaded base. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared overlay helper invoked once the work-object countdown reaches zero.
 * The same extern address and the same declaration are used by the exact
 * sibling incrementWorkByte9Until8ThenInvokeHelperAt101AndAdvanceWorkByte2
 * BmagicMagic07803_801F00EC, which publishes the 0x101 argument; this span
 * passes 0x102 from its own bytes. */
extern void func_8015D71C(u16 arg0); /* @source 0x8015D71C */

/* @source 0x801F01D0
 * @behavior Entry 4 (index 3, verified against the payload word at 0x801F08D0)
 *           of this overlay's six-word work-byte-2 handler pointer run based at
 *           0x801F08C4, whose in-image consumer func_801F0024 loads that run's
 *           base, scales the incoming index by four and calls the selected word
 *           through jalr, so this function has no jal caller and takes no
 *           arguments. It decrements the scratchpad work object's unsigned byte
 *           0xA through the pointer cell at 0x1F800044 as a byte-width
 *           read-modify-write (lbu / addiu -1 / sb) and tests the value it just
 *           stored (andi 0xFF of that register, empty branch delay slot) against
 *           zero. Only when that byte reaches zero it invokes the shared helper
 *           at 0x8015D71C with 0x102 and then re-materializes the cell with its
 *           own lui/lw pair and increments the work object's byte 2 at byte
 *           width (lbu / addiu 1 / sb), which moves the dispatcher to its next
 *           step. It returns nothing, keeps the 0x18-byte frame only to hold $ra
 *           at 0x10 across the call and reads or writes no other state.
 *
 *           Statement order, operand forms, offset-local spelling and
 *           declaration style follow two exact same-target siblings: the
 *           decrement guard and the post-arm work byte 2 advance come from
 *           decrementWorkByteAThenSetGlobalWorkByteBTo1AndAdvanceWorkByte2On
 *           ZeroBmagicMagic07803_801EF1CC, and the helper call with a
 *           literal argument followed by a fresh cell load and the byte 2
 *           increment come from incrementWorkByte9Until8ThenInvokeHelperAt101
 *           AndAdvanceWorkByte2BmagicMagic07803_801F00EC, whose post-call
 *           sequence is word-for-word this span's. This span's own bytes supply
 *           its 0xA decrement step, its zero test and its 0x102 argument. No
 *           payload under out/binaries repeats this 96-byte body: a byte scan
 *           of every extracted file found the span once, in this target's own
 *           payload, so the clone lever was ineligible.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAThenInvokeHelperAt102AndAdvanceWorkByte2OnZeroBmagicMagic07803_801F01D0(void) {
  if (--g_battle_work[0xA] == 0) {
    func_8015D71C(0x102);
    g_battle_work[2]++;
  }
}
