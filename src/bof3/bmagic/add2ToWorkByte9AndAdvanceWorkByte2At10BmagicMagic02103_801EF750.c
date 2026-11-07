#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays, bound at 0x1F800044 by this target's map. The cell is read once
 * per access, so the original re-materializes its address (lui $vN,0x1F80 /
 * lw $vN,68($vN)) before the byte-2 store group inside the taken branch. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF750
 * @behavior Adds two to scratchpad work-object byte 9 modulo 256 (lbu / addiu
 *           2 / sb), then tests the truncated byte it just stored (andi 0xFF of
 *           the stored register, no second load of the cell) against 0x10 for
 *           equality. Only when that byte is 0x10 does it re-read the pointer
 *           cell and increment the object's byte 2 modulo 256 (lbu / addiu 1 /
 *           sb), advancing to the next byte-2 handler step. Leaf with no frame
 *           and no saved registers: takes no arguments, returns nothing through
 *           the jr $ra at 0x801EF798 and reads or writes no other state.
 *           Reached by no in-image jal; it is entry 2 of the four-in-image-code-
 *           pointer run at 0x801EFE04 (0x801EF578, 0x801EF600, 0x801EF750,
 *           0x801EF7A0) that dispatchWorkByte1HandlerBmagicMagic02103_801EF534
 *           indexes with the work object's byte 1, so the handler-table entry
 *           proves the function-entry boundary at 0x801EF750. The roles of
 *           work-object bytes 9 and 2 are not established, so both stay
 *           offset-anchored. The reviewed 80-byte range is byte-identical to
 *           the exact siblings add2ToWorkByte9AndAdvanceWorkByte2At10 of
 *           emi/bmagic/magic131/03 (0x801F01A8) and emi/bmagic/magic222/03
 *           (0x801F061C); the body contains no relocation because its only
 *           address constant is the fixed scratchpad cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByte9AndAdvanceWorkByte2At10BmagicMagic02103_801EF750(void) {
  if ((g_battle_work[9] += 2) == 0x10) {
    g_battle_work[2]++;
  }
}
