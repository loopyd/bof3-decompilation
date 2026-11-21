#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. This target's map binds no
 * name for it, so it is declared here at its raw address. The compiled
 * original materializes the cell once (lui $v1,0x1F80 / lw $v1,0x44($v1),
 * load-delay nop) and keeps that single base for the guard and the increment,
 * with no aliasing reload between the two reads. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801EEDB0
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           (lui $v1,0x1F80 / lw $v1,0x44($v1), load-delay nop) and tests the
 *           work object's unsigned byte at offset 0xB against zero with a plain
 *           branch on the loaded byte (lbu $v0,0xB($v1) / bnez, empty delay
 *           slot), so the arm runs only when that byte is zero. In the arm it
 *           increments the work object's byte at offset 0x1 by one at byte
 *           width through the same base (lbu / addiu $v0,$v0,1 / sb), with no
 *           second cell load and no test of the stored sum. Takes no arguments,
 *           returns nothing, keeps no frame and touches no other state; the
 *           0x34-byte leaf ends in jr $ra with an empty return delay slot.
 *           The second entry of this overlay's leading pointer run at
 *           0x801EEC04 - the word at 0x801EEC08, read from payload offset 0x08
 *           - holds 0x801EEDB0, the dispatcher entry 0x801EEC10 selects with
 *           work byte 1, and no jal in the payload reaches it, which is the
 *           dispatch shape rather than a missing caller; the parameter roles
 *           therefore stay structural. One payload comparison put bytes
 *           0x801EEDB0..0x801EEDE3 equal to the exact lifts
 *           incrementWorkByte1WhenWorkByteBZeroBmagicMagic00903_801EF068
 *           (emi/bmagic/magic009/03) and
 *           incrementWorkByte1WhenWorkByteBZeroBmagicMagic14603_801EEFD0
 *           (emi/bmagic/magic146/03) and to the still-asm row 0x801EEEA8 of
 *           emi/bmagic/magic039/03, so this span is byte-identical to a lifted
 *           sibling and the clone lever supplied the statement order and
 *           operand forms; the two lifted siblings reach the same scratchpad
 *           cell through their map's g_battle_work row, which this target does
 *           not have, so the cell is spelled at its raw address here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1WhenWorkByteBZeroBmagicMagic14503_801EEDB0(void) {
  if (D_1F800044[0xB] == 0) {
    D_1F800044[1]++;
  }
}
