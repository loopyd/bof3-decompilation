#include "bof3/bof3.h"

/* Shared battle work-object pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt row D_801EB4E0 = 0x801EB4E0, so no target-map row is
 * needed, and the one access here is byte-addressed, so the local declaration
 * keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044 as g_battle_work, the name the exact
 * bmagic lifts of this family use for the same main-executable cell. The
 * compiled original re-materializes the cell at each of the three access groups
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)) because a byte store through the loaded
 * pointer may alias the cell, so every access stays on the named cell rather
 * than on a pointer local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF110
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044
 *           (lui $v1,0x1F80 / lw $v1,0x44($v1), load-delay nop) and decrements
 *           the work object's countdown byte at offset 0x9 at byte width
 *           (lbu $v0,0x9($v1) / addiu $v0,$v0,-1 / sb $v0,0x9($v1)), testing
 *           the stored value against zero (andi $v0,$v0,0xFF / bnez, whose
 *           delay slot materializes the compared one with
 *           addiu $v0,$zero,1); while that countdown stays above zero the
 *           function returns at once. When it reaches zero the function reads
 *           the shared battle work pointer at 0x801EB4E0 (lui $a1,0x1F80 /
 *           lw $a1,-0x4B20($a1), load-delay nop) and tests that record's byte
 *           at offset 0xB against one (lbu $a0,0xB($a1) / andi $v1,$a0,0xFF /
 *           bne $v1,$v0, with addiu $v0,$a0,-1 in the delay slot). When the byte
 *           equals one it re-reads the scratchpad cell and stores three into the
 *           work object's state byte at offset 0x2 (j with
 *           addiu $v0,$zero,3 in the delay slot / sb $v0,0x2($v1)); otherwise
 *           it stores the decremented 0xB back into that shared record
 *           (sb $v0,0xB($a1)), re-reads the scratchpad cell and advances the
 *           work object's byte at offset 0x2 by one at byte width (lbu /
 *           addiu $v0,$v0,1 / sb). Takes no arguments, returns nothing, keeps
 *           no frame and touches no other state. The word at 0x801EEC08 is
 *           entry 1 of this overlay's leading five-word handler run based at
 *           0x801EEC04 (0x801EEFE4, 0x801EF110, 0x801EF190, 0x801EF1C8,
 *           0x801EF2E4), which callWorkHandler2
 *           indexes with work byte 2; no jal in this payload reaches the span,
 *           which is the dispatch-table shape rather than a missing caller.
 *           Statement order, operand forms and declaration style follow the
 *           exact byte-9 countdown prefix of
 *           decrementWorkByte9ThenIncrementWorkByte1WhenZeroBmagicMagic00903
 *           _801EF01C, which decrements and zero-tests the same work-object
 *           countdown byte and then advances a re-read work byte, and the
 *           conditional body follows the exact guard-and-increment of
 *           incrementWorkByte2WhenGlobalWorkByteBIs1BmagicMagic08603_801F00C0
 *           with this function's own stored constant on the taken arm. No
 *           lifted sibling is byte-identical: an exhaustive word scan of every
 *           payload under out/binaries finds this body once, in this target.
 * @status exact
 * @match 100.00
 * @residual none
 */
void checkDownGlobalGlobal(void) {
  if (--g_battle_work[9] == 0) {
    if (D_801EB4E0[0xB] == 1) {
      g_battle_work[2] = 3;
    } else {
      D_801EB4E0[0xB]--;
      g_battle_work[2]++;
    }
  }
}
