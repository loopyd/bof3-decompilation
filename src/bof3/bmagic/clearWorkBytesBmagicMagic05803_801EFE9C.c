#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. This overlay's work-record updater func_801EF61C reads the same
 * cell (lui $a1,0x1F80 / lw $a1,68($a1)) before it accumulates the record's
 * words at offsets 0x0C and 0x18 and counts down its byte at 0x0A. The cell is
 * read once per access, so the original re-materializes its address
 * (lui $v0,0x1F80 / lw $v0,68($v0)) before each byte store. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFE9C
 * @behavior Clears the first five bytes (offsets 0x00-0x04) of the scratchpad
 * work record published through the pointer cell at 0x1F800044, storing $zero
 * with sb for each byte. The cell is read once per access, so the original
 * re-materializes the cell address and reloads the work pointer before every
 * store; the fifth load's delay slot holds the jr $ra and the fifth sb fills
 * the return delay slot. Leaf with no frame, no arguments and no return value;
 * it reads and writes no other state. Its only in-image reference is the jal
 * at 0x801EF684 in func_801EF61C, which reaches it after the work record's
 * byte at 0x0A counts down to zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytesBmagicMagic05803_801EFE9C(void) {
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
