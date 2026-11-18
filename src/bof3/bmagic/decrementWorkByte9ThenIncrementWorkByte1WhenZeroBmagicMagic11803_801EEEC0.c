#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The compiled original
 * re-materializes the symbol at both access groups
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)) because the byte store through the
 * loaded pointer may alias the cell, so the access stays on the named cell
 * rather than on a pointer local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEEC0
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044
 *           (lui $v1,0x1F80 / lw $v1,0x44($v1), load-delay nop) and
 *           decrements the work object's unsigned countdown byte at offset
 *           0x9 with a byte-width read-modify-write (lbu $v0,0x9($v1) /
 *           addiu $v0,$v0,-1 / sb $v0,0x9($v1)), testing the stored value
 *           against zero (andi $v0,$v0,0xFF / bnez, empty delay slot). Only
 *           when that countdown reaches zero it re-reads the cell (a second
 *           lui/lw pair) and increments the work object's work byte at
 *           offset 0x1 by one at byte width (lbu / addiu $v0,$v0,1 / sb).
 *           Takes no arguments, returns nothing, keeps no frame and touches
 *           no other state. The 0x4C-byte range is a leaf and keeps the
 *           original's empty delay slots: one after each of the two cell
 *           loads and one on the branch. The word at 0x801EEC10, the fourth
 *           entry of the overlay's leading pointer run at 0x801EEC04, holds
 *           this address; no jal in this payload reaches it, which is the
 *           dispatch shape rather than a missing caller. One payload cmp put
 *           bytes 0x801EEEC0..0x801EEF0B of this overlay equal to the
 *           0x4C-byte span 0x801EF01C..0x801EF067 of emi/bmagic/magic009/03,
 *           whose source
 *           decrementWorkByte9ThenIncrementWorkByte1WhenZeroBmagicMagic00903_801EF01C
 *           carried an exact match when this seed was written; its statement
 *           order and operand forms transfer here with this target's own
 *           g_battle_work binding, which both payloads address as the same
 *           scratchpad cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenIncrementWorkByte1WhenZeroBmagicMagic11803_801EEEC0(void) {
  if (--g_battle_work[9] == 0) {
    g_battle_work[1]++;
  }
}
