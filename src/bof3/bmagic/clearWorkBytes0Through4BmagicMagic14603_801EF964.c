#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. It holds the bmagic work record this
 * overlay steps: the caller func_801EF58C reads byte 0x09 and byte 0x0A of that
 * record through the same cell, increments the first and decrements the second
 * before it calls this function. The cell is read once per access, so the
 * original re-materializes its address (lui $v0,0x1F80 / lw $v0,68($v0)) before
 * each byte store. Other bmagic targets name the same cell through the same row. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF964
 * @behavior Clears work-record bytes 0x00 through 0x04 of the record published
 * through the scratchpad pointer cell at 0x1F800044, storing $zero with one sb
 * per byte. The cell is read once per access, so the original re-materializes
 * the cell address and reloads the work pointer before every store; the fifth
 * load's delay slot holds the jr $ra and the fifth sb fills the return delay
 * slot. Leaf with no frame, no arguments and no return value; it reads and
 * writes no other state. Its only in-image reference is the jal at 0x801EF5EC
 * in func_801EF58C, which increments the record's byte 0x09, decrements its
 * byte 0x0A and calls this function with that countdown reaching zero; the
 * countdown store sits in the call's delay slot. The 80-byte span is
 * byte-for-byte identical to emi/bmagic/magic219/03 0x801EF998,
 * emi/bmagic/magic223/03 0x801EFBDC and emi/bmagic/magic225/03 0x801F0338.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytes0Through4BmagicMagic14603_801EF964(void) {
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
