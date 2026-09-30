#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The cell is read once per access, so the original re-materializes
 * its address (lui $vN,0x1F80 / lw $vN,68($vN)) for each of the two accesses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF0C0
 * @behavior Stores 2 into the scratchpad work object's byte at offset 0x29
 * (materialized as li $v0,2 then sb), then re-reads the work-object pointer
 * cell at 0x1F800044 and increments that object's byte at offset 0x2 with a
 * byte-width add (lbu / addiu 1 / sb). Nothing is returned and no other state
 * is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte29AndIncrementWorkByte2BmagicMagic11103_801EF0C0(void) {
  g_battle_work[0x29] = 2;
  g_battle_work[2]++;
}
