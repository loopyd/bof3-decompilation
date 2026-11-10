#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound by this target's own map row
 * g_battle_work = 0x1F800044 and spelled the same way by this target's three
 * existing exact lifts. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF4DC
 * @behavior Entry 3 (index 3 of fourteen words) of the byte-1 handler table
 *           based at 0x801F046C, which dispatchStateByte1Table046cBmagicMagic11003_801EF3F8
 *           indexes with work record byte 1 and calls indirectly. Reads the
 *           scratchpad work pointer cell at 0x1F800044 once, decrements the
 *           unsigned byte 9 of the object with a byte-width read-modify-write
 *           (lbu / addiu -1 / sb) and compares the stored value with zero
 *           (andi 0xFF / bnez). Only when that byte reaches zero it re-reads
 *           the cell and increments the object's byte 2 with a byte-width add,
 *           advancing the work-byte-2 dispatcher to its next handler step. The
 *           second read of the cell is required: the store through the loaded
 *           work pointer may alias the cell. The function is a leaf with no
 *           frame; it takes no arguments, returns nothing and touches no other
 *           state. Bytes 0x801EF4DC..0x801EF527 are byte-identical to
 *           0x801EF394..0x801EF3DF in emi/bmagic/magic096/03 and to
 *           0x801EF194..0x801EF1DF in emi/bmagic/magic130/03, whose exact
 *           lifts decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic09603_801EF394
 *           and decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic13003_801EF194
 *           are the representatives this body is seeded from; their statement
 *           order, operand forms and declaration style are kept, and the
 *           scratchpad cell keeps this target's mapped spelling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic11003_801EF4DC(void) {
  if (--g_battle_work[9] == 0) {
    g_battle_work[2]++;
  }
}
