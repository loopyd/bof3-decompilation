#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The compiled original
 * re-materializes the symbol once (lui $v1,0x1F80 / lw $v1,0x44($v1)) and
 * keeps that single base for the guard and the increment, so this target-map
 * symbol is reached through one pointer load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF068
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           (lui $v1,0x1F80 / lw $v1,0x44($v1), load-delay nop) and tests the
 *           work object's byte at offset 0xB against zero with a plain branch
 *           on the loaded byte (lbu / bnez, empty delay slot), so the arm runs
 *           only when that byte is zero. In the arm it increments the work
 *           byte at offset 0x1 by one at byte width through the same base
 *           (lbu / addiu $v0,$v0,1 / sb), with no second cell load. Takes no
 *           arguments, returns nothing and keeps no frame. The word at
 *           0x801EEC10, the fourth entry of the overlay's leading pointer run,
 *           holds this address; no jal in this payload reaches it, which is the
 *           dispatch shape rather than a missing caller. Bytes
 *           0x801EF068..0x801EF09B are byte-identical to the unlifted
 *           0x801EEEA8 of bmagic/magic039/03, 0x801EEDB0 of bmagic/magic145/03
 *           and 0x801EEFD0 of bmagic/magic146/03, whose rows are still asm and
 *           own no source that could be cloned; the statement order and operand
 *           forms come from the exact sibling
 *           setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBZeroBmagicMagic07603_801EF11C
 *           without its store of 8 into the work byte at offset 0x9 and without
 *           its second cell load.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1WhenWorkByteBZeroBmagicMagic00903_801EF068(void) {
  if (g_battle_work[0xB] == 0) {
    g_battle_work[1]++;
  }
}
