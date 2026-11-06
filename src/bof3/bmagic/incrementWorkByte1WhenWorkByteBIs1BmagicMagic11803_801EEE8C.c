#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The original
 * re-materializes the symbol for one load (lui $a0,0x1F80 / lw $a0,0x44($a0))
 * and keeps that single base for the guard and the increment, which is the
 * form the exact sibling
 * incrementWorkByte1WhenWorkByteBIs1BmagicMagic04103_801EEFC4 uses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEE8C
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           (lui $a0,0x1F80 / lw $a0,0x44($a0), load-delay nop) and tests the
 *           work object's byte at offset 0xB against one (lbu / addiu
 *           $v0,$zero,1 / bne, empty delay slot). Only when that byte equals 1
 *           it increments the work byte at offset 0x1 by one at byte width
 *           through the same base (lbu / addiu $v0,$v0,1 / sb), with no second
 *           cell load. Takes no arguments, returns nothing and keeps no frame.
 *           The word at 0x801EEC0C, the third entry of the overlay's leading
 *           pointer run at 0x801EEC04, is the only in-image reference to this
 *           address; no jal in the payload reaches it, which is the dispatch
 *           shape rather than a missing caller. Bytes
 *           0x801EEE8C..0x801EEEBF are byte-identical to the exact
 *           0x801EEFC4 of bmagic/magic041/03, so the statement order and
 *           operand forms come from
 *           incrementWorkByte1WhenWorkByteBIs1BmagicMagic04103_801EEFC4 with
 *           this target's own g_battle_work binding.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1WhenWorkByteBIs1BmagicMagic11803_801EEE8C(void) {
  if (g_battle_work[0xB] == 1) {
    g_battle_work[1]++;
  }
}
