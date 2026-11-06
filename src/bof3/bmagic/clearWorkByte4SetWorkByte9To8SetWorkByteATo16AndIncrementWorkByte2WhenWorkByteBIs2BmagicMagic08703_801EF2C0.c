#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers. The original materialises the cell address once per group
 * (lui $a0/$v1,0x1F80 / lw ...,68(...)), so the cell is reached through
 * this target-map symbol instead of a constant address, which the compiler
 * folds once into a full-address register. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF2C0
 * @behavior Step of the work-object handler this overlay dispatches through the
 *           pointer runs at 0x801F0508 and 0x801F050C. It reads the scratchpad
 *           work-object pointer cell at 0x1F800044 and tests the work object's
 *           byte 0xB against 2 with a plain lbu/bne, and the same loaded pointer
 *           carries the arm's first store, so the guard byte and that store come
 *           from one cell read. In the arm it stores 0 into work byte 4, then
 *           re-reads the cell for each later group: byte 9 is set to 8 - the
 *           constant is materialized before the byte-4 store and fills the guard
 *           branch delay slot - byte 0xA is set to 0x10, and byte 2, the state
 *           byte the 0x801F050C dispatcher indexes, is advanced by one with a
 *           byte-width add (lbu / addiu 1 / sb). Leaf with no frame; takes no
 *           arguments, returns nothing and touches no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte4SetWorkByte9To8SetWorkByteATo16AndIncrementWorkByte2WhenWorkByteBIs2BmagicMagic08703_801EF2C0(
    void) {
  if (g_battle_work[0xB] == 2) {
    g_battle_work[4] = 0;
    g_battle_work[9] = 8;
    g_battle_work[0xA] = 0x10;
    g_battle_work[2]++;
  }
}
