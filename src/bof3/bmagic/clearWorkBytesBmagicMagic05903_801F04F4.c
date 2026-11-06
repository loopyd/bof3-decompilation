#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. It holds the record being stepped: the dispatch shape of this
 * overlay reads its byte +1 to index the local handler tables at 0x801F066C
 * and 0x801F0680. The cell is read once per access, so the original
 * re-materializes its address (lui $v0,0x1F80 / lw $v0,68($v0)) before each
 * byte store. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F04F4
 * @behavior Clears the first five bytes (offsets 0x00-0x04) of the scratchpad
 * work record published through the pointer cell at 0x1F800044, storing $zero
 * with sb for each byte. The cell is read once per access, so the original
 * re-materializes the cell address and reloads the work pointer before every
 * store; the last sb sits in the return delay slot. Takes no arguments,
 * returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytesBmagicMagic05903_801F04F4(void) {
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
