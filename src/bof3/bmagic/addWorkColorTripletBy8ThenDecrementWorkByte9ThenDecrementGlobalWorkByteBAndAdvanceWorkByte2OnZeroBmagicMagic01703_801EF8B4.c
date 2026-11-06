#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image; Splat
 * binds the address through this encoded name, which is why the original folds
 * the access to lui plus %lo. The sibling bmagic lifts read the same record:
 * decrementMotionCountdownAdjustMotionPairThenDecrementGlobalWorkByteBAndAdvance
 * WorkByte2BmagicMagic01500_801EFE84 decrements its byte 0xB. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named fixed address so
 * every access re-materializes the cell with its own lui plus offset load,
 * which is how the original reaches it once per group. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EF8B4
 * @behavior Per-frame handler of this overlay's handler run, entry 12: the
 *           code-pointer table that starts at 0x801EEC04 stores this function's
 *           address at 0x801EEC30. It adds 8 to the scratchpad work object's
 *           three colour channels at bytes 0x5D/0x5E/0x5F, reading the
 *           work-object pointer cell at 0x1F800044 once per channel, then
 *           decrements that object's countdown byte 9 with a byte-width
 *           read-modify-write (lbu / addiu -1 / sb) and tests the truncated
 *           stored value against zero (andi 0xFF / bnez). Only when the
 *           countdown reaches zero it decrements byte 0xB of the shared battle
 *           work record read through 0x801EB4E0, re-reads the work-object
 *           pointer cell and increments the object's handler byte 2 with a
 *           byte-width add, advancing the dispatcher on work byte 2 to its next
 *           handler step. The cell is re-read before that final group because a
 *           store through the record pointer may alias it. Leaf with no frame;
 *           takes no arguments and returns nothing, and reads or writes no
 *           other state. Bytes 0xD24..0xD67 are byte-identical to the complete
 *           exact sibling decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagic
 *           Magic09603_801EF394, whose statement order and operand forms the
 *           countdown group and the final group follow; the colour group copies
 *           addWorkColorTripletBy16ThenDecrementGlobalWorkByteBAndAdvanceWork
 *           Byte2BmagicMagic01500_801F1180 and writes this function's own
 *           delta 8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkColorTripletBy8ThenDecrementWorkByte9ThenDecrementGlobalWorkByteBAndAdvanceWorkByte2OnZeroBmagicMagic01703_801EF8B4(void) {
  D_1F800044[0x5D] += 8;
  D_1F800044[0x5E] += 8;
  D_1F800044[0x5F] += 8;

  if (--D_1F800044[9] == 0) {
    D_801EB4E0[0xB]--;
    D_1F800044[2]++;
  }
}
