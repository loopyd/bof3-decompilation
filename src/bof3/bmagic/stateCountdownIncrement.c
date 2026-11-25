#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original re-materializes the cell address for every
 * access, so the cell is reached through this target-map symbol rather than
 * through a constant address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EFD00
 * @behavior Adds one to scratchpad work-object byte 9 modulo 256 (lbu / addiu
 *           1 / sb), then tests the truncated byte it just stored (andi 0xFF of
 *           the stored register, no second load of the cell) against 0x10 for
 *           equality. Only when that byte is 0x10 does it re-read the pointer
 *           cell with its own lui/lw pair and increment the object's byte 2
 *           modulo 256 (lbu / addiu 1 / sb), advancing the dispatcher at
 *           0x801EFC58 to the next byte-2 handler step. Leaf with no frame and
 *           no saved registers: takes no arguments, returns nothing through the
 *           jr $ra at 0x801EFD48 and reads or writes no other state. Reached by
 *           no in-image jal; it is entry index 1 of the four-in-image-code
 *           pointer run at 0x801F0114 (0x801EFCC0, 0x801EFD00, 0x801EFD50,
 *           0x801EFD9C) that the byte-2 step table consumer indexes, so the
 *           entry proves the function-entry boundary at 0x801EFD00. The roles
 *           of work-object bytes 9 and 2 are not established, so both stay
 *           offset-anchored. One payload cmp of the 80-byte range proved it
 *           differs from the exact lift
 *           add2ToWorkByte9AndAdvanceWorkByte2At10BmagicMagic02103_801EF750 at
 *           byte 0x14 alone, the low byte of the addiu immediate (1 versus 2),
 *           so its statement form, register choice and operand spelling are
 *           copied unchanged with this span's own literal; the body contains no
 *           relocation because its only address constant is the fixed
 *           scratchpad cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCountdownIncrement(void) {
  if ((g_battle_work[9] += 1) == 0x10) {
    g_battle_work[2]++;
  }
}
