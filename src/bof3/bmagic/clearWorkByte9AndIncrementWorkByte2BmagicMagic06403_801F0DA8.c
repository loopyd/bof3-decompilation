#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The cell is read once per access, so the original re-materializes
 * its address (lui $vN,0x1F80 / lw $vN,68($vN)) for each of the two accesses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0DA8
 * @behavior Clears the scratchpad work-object byte at offset 0x9 (the original
 * stores $zero with sb) and then re-reads the work-object pointer cell at
 * 0x1F800044 and increments that object's byte at offset 0x2 with a byte-width
 * add (lbu, addiu $v0,$v0,1, sb scheduled into the $ra delay slot). Nothing is
 * returned, no other state is read or written and the function has no frame.
 * The 48 bytes are instruction-for-instruction identical to the exact sibling
 * clearWorkByte9AndIncrementWorkByte2BmagicMagic13103_801F0178
 * (emi/bmagic/magic131/03@0x801F0178), which uses the same two-statement shape.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9AndIncrementWorkByte2BmagicMagic06403_801F0DA8(void) {
  g_battle_work[9] = 0;
  g_battle_work[2]++;
}
