#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. Declared as a named symbol so each read is a lui plus offset load: the
 * original re-materializes the cell address before every store, because a
 * store through the loaded work pointer may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF4C4
 * @behavior Step handler published at index 1 of the work byte 2 handler
 * table based at 0x801F066C. It clears the scratchpad work object's byte 0x9
 * and byte 0xA with byte-width zero stores through the pointer cell at
 * 0x1F800044, re-materializing the cell before each store, then re-reads the
 * cell once more, reads work byte 0x2, adds one at byte width and stores the
 * result back. That read-modify-write runs the same way whether or not the
 * counter it clears was non-zero, so the step always advances the byte 0x2
 * dispatcher to the next handler. Leaf with no frame: the closing store sits
 * in the return delay slot. Takes no arguments, returns nothing and reads or
 * writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytes9AThenAdvanceWorkByte2BmagicMagic05903_801EF4C4(void) {
  g_battle_work[9] = 0;
  g_battle_work[10] = 0;
  g_battle_work[2]++;
}
