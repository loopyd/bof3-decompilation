#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through the encoded symbol name, so no target-map row or
 * WEAK_SYMBOL_AT binding is needed. The sibling bmagic lifts read the same
 * record through this pointer: seedWorkMotionFieldsWhenGlobalWorkByteBIs1
 * BmagicMagic01500_801EFD7C guards on its byte 0xB and
 * decrementMotionCountdownAdjustMotionPairThenDecrementGlobalWorkByteBAndAdvance
 * WorkByte2BmagicMagic01500_801EFE84 decrements it. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before each of the four access groups. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F1180
 * @behavior Entry 37 of the overlay handler table at 0x801EEC04, the word
 *           stored at 0x801EEC98. Per frame it adds 0x10 to the scratchpad work
 *           object's three colour channels at bytes 0x5D/0x5E/0x5F - the
 *           r0/g0/b0 triplet that entry 0x801F0F2C clears and the scenario
 *           lifts copy into SPRT fields - reading the work-object pointer cell
 *           at 0x1F800044 once per channel, and then reads the channel at 0x5D
 *           back through a signed byte view. When that value equals -0x40 (0xC0
 *           unsigned) it decrements byte 0xB of the shared battle work record
 *           read through 0x801EB4E0, re-reads the work-object pointer cell and
 *           increments the work object's handler byte 0x2 with a byte-width add
 *           (lbu / addiu 1 / sb), advancing to the next handler. The cell is
 *           re-read before that final group because a store through the record
 *           pointer may alias it. Leaf with no frame; takes no arguments and
 *           returns nothing, and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkColorTripletBy16ThenDecrementGlobalWorkByteBAndAdvanceWorkByte2BmagicMagic01500_801F1180(void) {
  D_1F800044[0x5D] += 0x10;
  D_1F800044[0x5E] += 0x10;
  D_1F800044[0x5F] += 0x10;

  if (((s8 *)D_1F800044)[0x5D] == -0x40) {
    D_801EB4E0[0xB]--;
    D_1F800044[2]++;
  }
}
