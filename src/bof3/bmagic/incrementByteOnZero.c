#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The compiled original
 * materializes the cell once (lui $v1,0x1F80 / lw $v1,0x44($v1)) and keeps
 * that single base for both the guard and the increment, so the map symbol is
 * reached through one pointer load and no aliasing reload. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEEA8
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           (lui $v1,0x1F80 / lw $v1,0x44($v1), load-delay nop) and tests the
 *           work object's unsigned byte at offset 0xB against zero with a
 *           plain branch on the loaded byte (lbu $v0,0xB($v1) / bnez, empty
 *           delay slot), so the arm runs only when that byte is zero. In the
 *           arm it increments the work object's work byte at offset 0x1 by one
 *           at byte width through the same base (lbu / addiu $v0,$v0,1 / sb),
 *           with no second cell load and no test of the stored sum. Takes no
 *           arguments, returns nothing, keeps no frame and touches no other
 *           state; the 0x34-byte leaf ends in jr $ra with an empty return
 *           delay slot. The word at 0x801EEC08, the second entry of the
 *           leading pointer run at 0x801EEC04, holds this address and is the
 *           only word in the payload that names it; no jal reaches it, which
 *           is the dispatch shape rather than a missing caller, so the
 *           parameter roles stay structural. One payload comparison put bytes
 *           0x801EEEA8..0x801EEEDB equal to 0x801EEFD0..0x801EEFF3 of
 *           emi/bmagic/magic146/03, to 0x801EF068..0x801EF09B of
 *           emi/bmagic/magic009/03 and to 0x801EEDB0..0x801EEDE3 of
 *           emi/bmagic/magic145/03; the statement order and operand forms are
 *           the magic146/03 source
 *           incrementWorkByte1WhenWorkByteBZeroBmagicMagic14603_801EEFD0,
 *           whose row was lifted exact, and all four payloads read 0x1F800044
 *           as the same scratchpad cell that this target's map binds as
 *           g_battle_work.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementByteOnZero(void) {
  if (g_battle_work[0xB] == 0) {
    g_battle_work[1]++;
  }
}
