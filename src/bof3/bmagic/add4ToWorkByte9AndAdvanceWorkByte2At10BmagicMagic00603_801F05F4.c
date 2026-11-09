#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays, bound at 0x1F800044 by this target's map. The cell is read once
 * per access, so the original re-materializes its address (lui $vN,0x1F80 /
 * lw $vN,68($vN)) before the byte-2 store group inside the taken branch. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F05F4
 * @behavior Adds four to scratchpad work-object byte 9 modulo 256 (lbu /
 *           addiu 4 / sb), then tests the truncated byte it just stored
 *           (andi 0xFF of the stored register, no second load of the cell)
 *           against 0x10 for equality. Only when that byte is 0x10 does it
 *           re-read the pointer cell with its own lui/lw pair and increment
 *           the object's byte 2 modulo 256 (lbu / addiu 1 / sb), advancing to
 *           the next byte-2 handler step. Leaf with no frame and no saved
 *           registers: takes no arguments, returns nothing through the
 *           jr $ra at 0x801F063C and reads or writes no other state. Reached
 *           by no in-image jal; the word at 0x801EEC2C is entry 10 of the
 *           13-word code-pointer run 0x801EEC04..0x801EEC34 in this target's
 *           tabledata segment, so the handler-table entry proves the
 *           function-entry boundary at 0x801F05F4, and its neighbour at
 *           0x801EEC28 is this target's own exact lift
 *           copyWorkWordTripletClearByte9ThenAdvanceWorkByte2BmagicMagic00603
 *           _801F05A4. The roles of work-object bytes 9 and 2 are not
 *           established, so both stay offset-anchored. The reviewed 80-byte
 *           range differs from the exact lifts
 *           add2ToWorkByte9AndAdvanceWorkByte2At10 of emi/bmagic/magic021/03
 *           (0x801EF750), emi/bmagic/magic131/03 (0x801F01A8) and
 *           emi/bmagic/magic222/03 (0x801F061C) in exactly one byte, the low
 *           byte of the addiu immediate (2 versus 4), so the statement form
 *           and register choice are taken from those twins unchanged; the
 *           body contains no relocation because its only address constant is
 *           the fixed scratchpad cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add4ToWorkByte9AndAdvanceWorkByte2At10BmagicMagic00603_801F05F4(void) {
  if ((g_battle_work[9] += 4) == 0x10) {
    g_battle_work[2]++;
  }
}
