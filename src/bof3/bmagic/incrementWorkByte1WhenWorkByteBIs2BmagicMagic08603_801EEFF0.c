#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The original
 * re-materializes the symbol for one load (lui $a0,0x1F80 / lw $a0,0x44($a0))
 * and keeps that single base for the guard and the increment, the form the
 * exact siblings incrementWorkByte1WhenWorkByteBIs1BmagicMagic04103_801EEFC4
 * and incrementWorkByte1WhenWorkByteBIs1BmagicMagic11803_801EEE8C use. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEFF0
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           (lui $a0,0x1F80 / lw $a0,0x44($a0), load-delay nop) and tests the
 *           work object's byte at offset 0xB against two (lbu / addiu
 *           $v0,$zero,2 / bne, empty delay slot). Only when that byte equals 2
 *           it increments the work byte at offset 0x1 by one at byte width
 *           through the same base (lbu / addiu $v0,$v0,1 / sb), with no second
 *           cell load. Takes no arguments, returns nothing and keeps no frame.
 *           The word at 0x801EEC10, the fourth entry of the overlay's leading
 *           pointer run at 0x801EEC04, is the only in-image reference to this
 *           address; no jal in the payload reaches it, which is the dispatch
 *           shape rather than a missing caller, and 0x801EEFF0 is the handler
 *           the overlay entry point at 0x801EEC1C selects with the work byte
 *           at offset 0x1. The 52-byte body is instruction-for-instruction
 *           identical to the two exact bodies above, differing only in the
 *           compared immediate at 0x801EF000 (2 here, 1 there), so the
 *           statement order and operand forms come from that sibling shape
 *           with this function's own constant; no other payload under
 *           out/binaries holds these bytes, so no byte-identical twin exists
 *           and the clone lever is ineligible.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1WhenWorkByteBIs2BmagicMagic08603_801EEFF0(void) {
  if (g_battle_work[0xB] == 2) {
    g_battle_work[1]++;
  }
}
