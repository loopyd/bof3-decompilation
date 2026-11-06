#include "bof3/bof3.h"

/* Scratchpad pointer cell that holds the battle work record this overlay
 * steps. The cell is read once per access, so the original re-materializes its
 * address (lui $v0,0x1F80 / lw $v0,68($v0)) before each byte store. Other
 * sources of this target name the same cell through the same row. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFBDC
 * @behavior Clears work-record bytes 0x00 through 0x04 of the record published
 * through the scratchpad pointer cell at 0x1F800044, storing $zero with one sb
 * per byte. The cell is read once per access, so the original re-materializes
 * the cell address and reloads the work pointer before every store; the fifth
 * load's delay slot holds the jr $ra and the fifth sb fills the return delay
 * slot. Leaf with no frame, no arguments and no return value; it reads and
 * writes no other state. Its only in-image reference is the jal at 0x801EF790
 * in func_801EF6E0, which decrements byte 0x0B of the record published at
 * 0x801EB4E0 and calls this function while work byte 0x09 is 0x14. The 80-byte
 * span is byte-for-byte identical to emi/bmagic/magic225/03 0x801F0338.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytes0Through4BmagicMagic22303_801EFBDC(void) {
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
