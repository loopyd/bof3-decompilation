#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original re-materializes the cell address for every
 * access, so the cell is reached through this target-map symbol rather than
 * through a constant address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFCC0
 * @behavior Resets two bytes of the scratchpad work object and advances a
 * third: the object byte at offset 0x9 is cleared (the original stores $zero
 * with sb), the object byte at offset 0xA is set to the literal 0x88 (addiu
 * $v0,$zero,0x88 followed by sb) and the object byte at offset 0x2 is
 * incremented by one with a byte-width add (lbu, addiu $v0,$v0,1, sb; the
 * store is scheduled into the $ra delay slot). The work-object pointer cell at
 * 0x1F800044 is re-materialized and reloaded once per access, so the original
 * keeps three separate lui $vN,0x1F80 / lw $vN,0x44($vN) pairs. Nothing is
 * returned and no other state is read or written; the function has no frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9SetWorkByteATo88AndIncrementWorkByte2BmagicMagic13703_801EFCC0(void) {
  g_battle_work[9] = 0;
  g_battle_work[0xA] = 0x88;
  g_battle_work[2]++;
}
