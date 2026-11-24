#include "bof3/bof3.h"

/* Shared overlay helper invoked once the work-object script/state countdown
 * reaches zero; the same extern address is called by the exact sibling
 * incrementByteInvokeHelper of this
 * target. The call takes no arguments in the original (jal with no argument
 * setup). */
void func_801E5988(void);

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * This target's map binds it at 0x1F800044 as g_battle_work, the same binding
 * the exact siblings decrementWorkByteAThenSetGlobalWorkByteBTo1AndAdvance
 * WorkByte2OnZeroBmagicMagic07803_801EF1CC and add8ToWorkByteAThenAdvance
 * WorkByte2WhenAtLeast81BmagicMagic07803_801EF278 use. The original loads the
 * cell once (lui $v1,0x1F80 / lw $v1,0x44($v1)) before the 0x18-byte frame is
 * created and reaches the guarded byte 0x9 access through that one pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The remaining asm spans func_801EEF28 and func_801EF1CC still reference the
 * address, so Splat's generated undefined_syms_auto.txt binds it as
 * D_801EB4E0 and the byte-addressed store below needs the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF2CC
 * @behavior Entry 9 (index 8, the last word) of this overlay's nine-word
 *           work-byte-1 handler pointer run based at 0x801F0878, whose
 *           dispatcher at 0x801EEEE4 indexes that run on the work-object byte
 *           at offset 0x01, so this function has no jal caller. It decrements
 *           the scratchpad work object's unsigned byte 0x9 through one load of
 *           the pointer cell at 0x1F800044 (lbu / addiu -1 / sb) and tests the
 *           byte it just stored (andi 0xFF of the stored register, no second
 *           load of the cell) against zero. Only when that byte reaches zero
 *           does it read the shared battle work-record pointer at 0x801EB4E0,
 *           store 0xFF into that record's byte at offset 0xB - that constant is
 *           materialised in the delay slot of the zero test, ahead of the
 *           record load - and invoke the shared helper at 0x801E5988, the store
 *           filling the call delay slot. It does not advance the work object's
 *           byte 2 itself; the helper does. The 0x18-byte frame only keeps $ra
 *           at 0x10 because of that call, and the work pointer load is issued
 *           before the frame is created. Takes no arguments, returns nothing
 *           and reads or writes no other state.
 *
 *           Statement order, operand forms, offset-local spelling and
 *           declaration style follow two exact siblings: the same-target
 *           decrementWorkByteAThenSetGlobalWorkByteBTo1AndAdvanceWorkByte2On
 *           ZeroBmagicMagic07803_801EF1CC (same named pointer cell, same
 *           byte-width decrement guard, same shared record byte 0xB written
 *           inside the zero arm) and incrementWorkByte9ThenSetGlobalWorkByteB
 *           ToFFAndInvokeHelperWhenWorkByte9Is20BmagicMagic09403_801EF0FC
 *           (0xFF stored into the same record byte from the test delay slot
 *           with the helper call in the arm). This span's own bytes supply its
 *           guarded offset 0x9, its -1 step, its zero test and its stored
 *           0xFF, and they carry no work byte 2 advance, so no third statement
 *           is written in the arm. No payload under out/binaries repeats this
 *           76-byte body: a byte scan over every extracted file found the span
 *           once, in this target's own payload, so the clone lever is
 *           ineligible.
 * @status exact
 * @match 100.00
 * @residual none
 */
void mathSetGlobalHelper(void) {
  if (--g_battle_work[9] == 0) {
    D_801EB4E0[0xB] = 0xFF;
    func_801E5988();
  }
}
