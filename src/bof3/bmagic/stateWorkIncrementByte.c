#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. Declared as the named cell its exact siblings on this target use: the
 * symbolic address folds into each load (lui $v1,0x1F80 / lw $v1,68($v1)), and
 * the original re-reads the cell for the second store group because the store
 * that precedes it goes through the pointer the cell supplied. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF0C4
 * @behavior Overlay entry handler. Reads the scratchpad work-object pointer
 *           cell at 0x1F800044 and compares the work object's byte at offset
 *           0xB against two with an unsigned bound (lbu / sltiu 2), so the
 *           arm is taken for every value below two. Only on that path it
 *           stores the literal 8 into the work byte at offset 0x9, then
 *           re-reads the pointer cell and increments the work byte at offset
 *           0x1 by one at byte width (lbu / addiu 1 / sb); the second read
 *           exists because the store that precedes it goes through the
 *           pointer the cell supplied. Leaf with no frame: takes no arguments,
 *           returns nothing and touches no other state. No jal in this payload
 *           reaches it; its address is word 3 of the six-word overlay entry
 *           run at 0x801EEC04 (payload offset 0x10, after the leading 0x178
 *           word), which the loader calls, so its argument roles stay
 *           structural. Bytes 0x801EF0C4..0x801EF10B are byte-identical to the
 *           already exact sibling
 *           setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBBelow2BmagicMagic06203_801EF078,
 *           proven by a payload cmp of both 0x48-byte spans, and this body
 *           follows that sibling's statement order with this function's own
 *           address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWorkIncrementByte(
    void) {
  if (g_battle_work[0xB] < 2u) {
    g_battle_work[9] = 8;
    g_battle_work[1]++;
  }
}
