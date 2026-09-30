#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The cell is read once per access, so the original re-materializes
 * its address (lui $vN,0x1F80 / lw $vN,68($vN)) for each of the two accesses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0178
 * @behavior Clears the scratchpad work-object byte at offset 0x9 (the original
 * stores $zero with sb) and then increments the work-object byte at offset 0x2
 * with a byte-width add (lbu, addiu 1, sb). The pointer cell at 0x1F800044 is
 * read once per access, so the original re-materializes the cell address and
 * reloads the work pointer for the increment. Nothing is returned and no other
 * state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9AndIncrementWorkByte2BmagicMagic13103_801F0178(void) {
  g_battle_work[9] = 0;
  g_battle_work[2]++;
}
