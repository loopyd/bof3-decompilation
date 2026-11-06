#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. It holds the bmagic work record this
 * overlay steps: the caller func_801EF5DC reads byte 0x0A and byte 0x09 of that
 * record through the same cell and decrements both before it calls this
 * function. The cell is read once per access, so the original re-materializes
 * its address (lui $v0,0x1F80 / lw $v0,68($v0)) before each byte store. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF998
 * @behavior Clears the first five bytes (offsets 0x00-0x04) of the scratchpad
 * work record published through the pointer cell at 0x1F800044, storing $zero
 * with one sb per byte. The cell is read once per access, so the original
 * re-materializes the cell address and reloads the work pointer before every
 * store; the fifth load's delay slot holds the jr $ra and the fifth sb fills the
 * return delay slot. Leaf with no frame, no arguments and no return value; it
 * reads and writes no other state. Its only in-image reference is the jal at
 * 0x801EF64C in func_801EF5DC, which decrements the record's byte at offset
 * 0x09 and calls this function when that countdown reaches zero. The 80-byte
 * span is byte-for-byte identical to emi/bmagic/magic223/03 0x801EFBDC and
 * emi/bmagic/magic225/03 0x801F0338.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytes0Through4BmagicMagic21903_801EF998(void) {
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
