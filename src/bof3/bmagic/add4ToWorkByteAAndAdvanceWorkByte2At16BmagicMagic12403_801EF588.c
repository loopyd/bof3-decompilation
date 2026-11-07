#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings on this overlay family use: the symbolic address
 * folds into each load (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's two
 * separate reads each re-materialise the base. The plain cell is kept rather
 * than the volatile one several siblings use: the second read is already
 * separated from the first by a store through the loaded pointer, so the reload
 * comes from the store and not from a qualifier, and the sibling
 * seedMotionChannelTripletTintAndCountdownThenAdvanceWorkByte2
 * BmagicMagic15103_801EFE90 measured the volatile spelling identical on this
 * same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF588
 * @behavior Advances the scratchpad work object's byte at offset 0xA by four
 *           modulo 256 through one load of the pointer cell at 0x1F800044
 *           (lbu / addiu 4 / sb) and tests the truncated byte it just stored
 *           (andi 0xFF of the stored register, no second load of the cell) for
 *           equality with 0x10, the literal materialised into the register the
 *           equality test compares against. Only when that byte equals 0x10
 *           does it re-read the pointer cell and increment the object's byte at
 *           offset 0x2 (lbu / addiu 1 / sb), the overlay work byte 2 handler
 *           step. Leaf with no frame; takes no arguments, returns nothing and
 *           reads or writes no other state.
 *
 *           Shape: the store-then-test-the-stored-byte block follows the exact
 *           sibling add4ToWorkByteAThenSetWorkByteBToOneAndAdvanceWorkByte2
 *           WhenAtLeast20BmagicMagic15103_801F0C50, which adds four to this
 *           same work byte 0xA, stores the narrowed value local and then tests
 *           that stored value with its truncation immediate instead of
 *           reloading the cell. This original carries the equality test of
 *           add4ToWorkByte9AndAdvanceWorkByte2At40BmagicMagic13103_801F053C
 *           with only its guard arm, and it holds only the trailing byte 2
 *           increment of the 15103 sibling's arm.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add4ToWorkByteAAndAdvanceWorkByte2At16BmagicMagic12403_801EF588(void) {
  u8 *work;
  u8 value;

  work = g_battle_work;
  value = work[0xA] + 4;
  work[0xA] = value;
  if (value == 0x10) {
    g_battle_work[2]++;
  }
}
