#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name, so no target-map row or
 * WEAK_SYMBOL_AT binding is needed. The sibling bmagic lifts read the same
 * record: seedWorkMotionFieldsWhenGlobalWorkByteBIs1BmagicMagic01500_801EFD7C
 * guards on its byte 0xB and
 * seedWorkObjectFromBattleRecordAndAdvanceWorkByte2BmagicMagic04500_801EF4A4
 * copies its words at 0x34 and 0x38 and its halfword at 0x3E. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before the second store group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EFDE8
 * @behavior Per-frame motion step of one handler of this overlay's header
 *           handler table, the code-pointer run that starts at 0x801EEC04 and
 *           stores this function's address at 0x801EEC48, directly after
 *           seedWorkMotionFieldsWhenGlobalWorkByteBIs1BmagicMagic01500_801EFD7C
 *           and before
 *           decrementMotionCountdownAdjustMotionPairThenDecrementGlobalWorkByte
 *           BAndAdvanceWorkByte2BmagicMagic01500_801EFE84. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once and then
 *           works on the object in place: it decrements the countdown byte 0x9,
 *           subtracts 0x2000 from the 16.16 motion word at 0x40, adds 0x3000 to
 *           the motion word at 0x44, adds 0x20 to the halfword at 0x3E and
 *           stores the new countdown. Each field is a read-modify-write on the
 *           loaded pointer, so the original keeps one address in a register for
 *           all four updates. When the new countdown byte is zero it reads the
 *           shared battle work record through 0x801EB4E0, re-reads the
 *           scratchpad cell, copies the record's words at 0x34, 0x38 and 0x3C
 *           into the same offsets of the work object, resets work byte 9 to 8,
 *           and advances the work object's handler byte 0x2 with a byte-width
 *           add (lbu / addiu 1 / sb). The cell is re-read before that group
 *           because a store through the record pointer may alias it; the 0x3C
 *           word is held in a local so its store follows the countdown reset,
 *           and that store's scheduling into the branch delay slot is the
 *           original's. Leaf with no frame; takes no arguments and returns
 *           nothing, and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementMotionCountdownAdjustMotionPairAndHalfwordThenCopyRecordTripletResetCountdownAndAdvanceWorkByte2BmagicMagic01500_801EFDE8(void) {
  u8 *work;
  u8 counter;
  u8 *source;
  u8 *copy;
  u32 position;
  u8 *step;

  work = D_1F800044;
  counter = work[9] - 1;
  *(s32 *)(work + 0x40) -= 0x2000;
  *(s32 *)(work + 0x44) += 0x3000;
  *(u16 *)(work + 0x3E) += 0x20;
  work[9] = counter;
  if (counter == 0) {
    source = D_801EB4E0;
    copy = D_1F800044;
    *(u32 *)(copy + 0x34) = *(u32 *)(source + 0x34);
    *(u32 *)(copy + 0x38) = *(u32 *)(source + 0x38);
    position = *(u32 *)(source + 0x3C);
    copy[9] = 8;
    step = D_1F800044;
    *(u32 *)(copy + 0x3C) = position;
    step[2]++;
  }
}
