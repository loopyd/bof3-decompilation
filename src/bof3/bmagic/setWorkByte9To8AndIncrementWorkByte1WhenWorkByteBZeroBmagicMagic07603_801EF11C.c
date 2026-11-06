#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original re-materializes and reloads the cell
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)) after the arm's first store, so the cell
 * is reached through this target-map symbol rather than through a constant
 * address, which the compiler materializes once and reuses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF11C
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 and
 *           tests the work object's byte at offset 0xB against zero with a
 *           plain branch on the loaded byte (lbu/bnez), so the arm runs only
 *           when that byte is zero. In the arm it stores 8 into the work byte
 *           at offset 0x9, then re-reads the cell and increments the work byte
 *           at offset 0x1 by one at byte width (lbu, addiu $v0,$v0,1, sb).
 *           Takes no arguments, returns nothing and keeps no frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBZeroBmagicMagic07603_801EF11C(
    void) {
  if (g_battle_work[0xB] == 0) {
    g_battle_work[9] = 8;
    g_battle_work[1]++;
  }
}
