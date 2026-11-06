#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The original
 * re-materializes the symbol for one load (lui $a0,0x1F80 / lw $a0,0x44($a0))
 * and keeps that single base for the guard and the increment, the form the
 * exact sibling incrementWorkByte1WhenWorkByteBBelow2BmagicMagic08803_801EF070
 * uses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEFC4
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           and tests the work object's byte at offset 0xB against one
 *           (lbu / addiu $v0,$zero,1 / bne, empty delay slot). Only when that
 *           byte equals 1 it increments the work byte at offset 0x1 by one at
 *           byte width through the same base (lbu / addiu / sb), which is the
 *           handler index the overlay step dispatcher at 0x801EF190 advances
 *           through the table at 0x801EFD8C. Takes no arguments, returns
 *           nothing and keeps no frame. The word at 0x801EEC10 of the
 *           overlay's leading pointer run is the only in-image reference to
 *           this address, and it is the dispatch shape, not a jal caller.
 *           0x801EEFC4..0x801EEFF7 are byte-identical to the unlifted
 *           0x801EEE8C of bmagic/magic118/03, whose row is still asm and which
 *           owns no source that could be cloned; the statement order and
 *           operand forms come from the exact sibling
 *           incrementWorkByte1WhenWorkByteBIs80BmagicMagic11303_801EF3A0 with
 *           this function's own compared constant.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1WhenWorkByteBIs1BmagicMagic04103_801EEFC4(void) {
  if (g_battle_work[0xB] == 1) {
    g_battle_work[1]++;
  }
}
