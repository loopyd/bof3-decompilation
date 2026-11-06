#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. It holds the bmagic work record this
 * overlay steps: the caller func_801F021C reads the same cell and updates that
 * record's byte at offset 0x09 and the words at offsets 0x34, 0x38, 0x40 and
 * 0x44 through it. The cell is read once per access, so the original
 * re-materializes its address (lui $v0,0x1F80 / lw $v0,68($v0)) before each
 * byte store. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0338
 * @behavior Clears the first five bytes (offsets 0x00-0x04) of the scratchpad
 * work record published through the pointer cell at 0x1F800044, storing $zero
 * with one sb per byte. The cell is read once per access, so the original
 * re-materializes the cell address and reloads the work pointer before every
 * store; the fifth load's delay slot holds the jr $ra and the fifth sb fills
 * the return delay slot. Leaf with no frame, no arguments and no return value;
 * it reads and writes no other state. Its only in-image reference is the jal
 * at 0x801F02BC in func_801F021C, which decrements the record's byte at offset
 * 0x09 and calls this function when that countdown reaches zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytesBmagicMagic22503_801F0338(void) {
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
