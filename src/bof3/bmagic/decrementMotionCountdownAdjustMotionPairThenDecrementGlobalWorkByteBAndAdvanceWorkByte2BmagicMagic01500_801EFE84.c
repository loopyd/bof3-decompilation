#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name, so no target-map row or
 * WEAK_SYMBOL_AT binding is needed. The sibling bmagic lifts read the same
 * record: seedWorkMotionFieldsWhenGlobalWorkByteBIs1BmagicMagic01500_801EFD7C
 * guards on its byte 0xB and
 * seedWorkWordsAndCopyRecordByte27ThenAdvanceWorkByte2BmagicMagic01500_801F0D48
 * copies its byte 0x27. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before its second store group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EFE84
 * @behavior Per-frame motion step of one handler of this overlay's header
 *           handler table, the code-pointer run that starts at 0x801EEC04 and
 *           stores this function's address at 0x801EEC4C. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once, loads the
 *           object's countdown byte 0x9 and the 16.16 motion pair words at
 *           offsets 0x40 and 0x44 (the pair that
 *           seedWorkMotionFieldsWhenGlobalWorkByteBIs1BmagicMagic01500_801EFD7C
 *           seeds with 0x10000), decrements the countdown byte by one, adds
 *           0x2000 to the word at 0x40 and subtracts 0x3000 from the word at
 *           0x44, and writes the countdown byte back with sb and the two
 *           adjusted words back with sw, the second of which the original
 *           schedules into the branch delay slot. When the new countdown byte
 *           is zero it decrements byte 0xB of the shared battle work record
 *           read through 0x801EB4E0, re-reads the scratchpad pointer cell and
 *           increments the work object's handler byte 0x2 with a byte-width add
 *           (lbu / addiu 1 / sb), advancing to the next handler. The cell is
 *           re-read because a store through the record pointer may alias it.
 *           Leaf with no frame; takes no arguments and returns nothing, and
 *           reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementMotionCountdownAdjustMotionPairThenDecrementGlobalWorkByteBAndAdvanceWorkByte2BmagicMagic01500_801EFE84(void) {
  u8 *work;
  u8 counter;
  s32 word40;
  s32 word44;

  work = D_1F800044;
  counter = work[9] - 1;
  word40 = *(s32*)(work + 0x40) + 0x2000;
  word44 = *(s32*)(work + 0x44) - 0x3000;
  work[9] = counter;
  *(s32*)(work + 0x40) = word40;
  *(s32*)(work + 0x44) = word44;
  if (counter == 0) {
    D_801EB4E0[0xB]--;
    D_1F800044[2]++;
  }
}
