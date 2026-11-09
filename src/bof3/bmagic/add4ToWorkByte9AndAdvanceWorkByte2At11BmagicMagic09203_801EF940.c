#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. A store through the loaded pointer may alias the cell, so the original
 * re-materializes the cell address (lui $a0,0x1F80 / lw $a0,68($a0)) after the
 * byte-9 store and reuses that reload for the byte-9 test and the byte-2
 * increment. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF940
 * @behavior Adds four to scratchpad work-object byte 9 modulo 256 (lbu /
 *           addiu 4 / sb) through the first read of the work pointer cell
 *           0x1F800044, then re-reads the cell and the byte it just stored;
 *           only when that byte equals 0x11 does the reloaded pointer
 *           increment work byte 2 modulo 256 (lbu / addiu 1 / sb), which
 *           advances the byte-2 handler step. Leaf with no frame and no saved
 *           registers: takes no arguments, returns nothing through the
 *           jr $ra at 0x801EF98C, and reads or writes no other state. Reached
 *           by no in-image jal; the word at 0x801F07DC is entry 8 of the
 *           12-word handler table based at 0x801F07BC, which the byte-1
 *           dispatcher at 0x801EF000 indexes, so the table entry proves the
 *           function-entry boundary at 0x801EF940. The roles of work-object
 *           bytes 9 and 2 are not established and stay offset-anchored. The
 *           reviewed 80-byte range differs from the exact lift
 *           add4ToWorkByte9AndAdvanceWorkByte2At40 of emi/bmagic/magic131/03
 *           (0x801F053C) in the addiu immediate of the equality test alone
 *           (0x11 versus 0x40), so the statement form, the reload of the work
 *           pointer cell and the register choice are taken from that twin
 *           unchanged; the body contains no relocation because its only
 *           address constant is the fixed scratchpad cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add4ToWorkByte9AndAdvanceWorkByte2At11BmagicMagic09203_801EF940(void) {
  g_battle_work[9] += 4;
  if (g_battle_work[9] == 0x11) {
    g_battle_work[2]++;
  }
}
