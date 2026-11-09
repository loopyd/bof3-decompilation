#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is a named cell
 * symbol, the form this target's exact siblings use
 * (copyWorkWordTripletAndClearByte9ThenAdvanceWorkByte2BmagicMagic04103_801EF484,
 * incrementWorkByte2WhenGlobalWorkByteBIs1BmagicMagic04103_801EF524): the
 * symbolic address lets the assembler fold it into each load
 * (lui $vN,0x1F80 / lw $vN,68($vN)). The span re-materializes the address for
 * the taken arm, so the cell is read twice and never cached across the guard. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF4D4
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           and advances that object's unsigned byte 9 by two modulo 256
 *           (lbu $v1,9($v0) / addiu $v1,$v1,2 / sb $v1,9($v0)). It then tests
 *           the register it just stored, masked to a byte with andi 0xFF and
 *           with no second load of the cell, for equality with 0x10 (addiu
 *           $v0,$zero,16 / bne). Only on equality it re-materializes the cell
 *           with its own lui/lw pair and increments the object's byte 2 modulo
 *           256 (lbu $v0,2($v1) / addiu $v0,$v0,1 / sb $v0,2($v1)), advancing
 *           to the next byte-2 handler step. Leaf with no frame and no saved
 *           registers: takes no arguments, returns nothing through the jr $ra
 *           at 0x801EF520 and reads or writes no other state. No in-image jal
 *           reaches it; the entry is word 11 of the pre-image entry table the
 *           overlay header carries at payload offsets 0x04..0x34, which holds
 *           0x801EF4D4 at offset 0x2C and proves the entry boundary, matching
 *           the same table word of the exact siblings. The roles of work-object
 *           bytes 9 and 2 are not established, so both stay offset-anchored.
 *           The reviewed 80-byte range measured byte-identical to the
 *           then-exact siblings add2ToWorkByte9AndAdvanceWorkByte2At10 of
 *           emi/bmagic/magic021/03 (0x801EF750), emi/bmagic/magic131/03
 *           (0x801F01A8) and emi/bmagic/magic222/03 (0x801F061C); the body
 *           carries no relocation because its only address constant is the
 *           fixed scratchpad cell, and the hand copy keeps this target's
 *           evidenced cell name where the clone adapter would write the raw
 *           address spelling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByte9AndAdvanceWorkByte2At10BmagicMagic04103_801EF4D4(void) {
  if ((g_battle_work[9] += 2) == 0x10) {
    g_battle_work[2]++;
  }
}
