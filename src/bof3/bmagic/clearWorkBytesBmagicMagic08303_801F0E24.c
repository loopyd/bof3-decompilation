#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The cell is read once per access, so the original re-materializes
 * its address (lui $v0,0x1F80 / lw $v0,68($v0)) before each byte store. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0E24
 * @behavior Clears the first five bytes (offsets 0x00-0x04) of the scratchpad
 * work record published through the pointer cell at 0x1F800044, storing $zero
 * with sb for each byte. The cell is read once per access, so the original
 * re-materializes the cell address and reloads the work pointer before every
 * store. Nothing is returned and no other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytesBmagicMagic08303_801F0E24(void) {
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
