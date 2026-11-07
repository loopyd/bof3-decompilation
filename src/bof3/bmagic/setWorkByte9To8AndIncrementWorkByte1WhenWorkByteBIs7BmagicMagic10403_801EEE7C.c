#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The original
 * re-materializes the symbol for the guard load (lui $a0,0x1F80 /
 * lw $a0,0x44($a0), load-delay nop) and keeps that single base for the guard
 * and the 0x9 store, then reloads the cell (lui $v1,0x1F80 / lw $v1,0x44($v1))
 * for the 0x1 increment because the store through the base may alias the cell
 * itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEE7C
 * @behavior Stage handler at index 1 of the four-word code pointer run at
 *           0x801EEC04, the payload words 0x801EEC80, 0x801EEE7C, 0x801EEEC0
 *           and 0x801EF03C that the overlay entry at 0x801EEC14 copies into its
 *           0x28-byte frame and selects with work byte 0x1 (lbu / sll 2 /
 *           addu / lw / jalr). It reads the scratchpad work-object pointer cell
 *           at 0x1F800044 once and tests the work object's byte at offset 0xB
 *           against seven by equality (lbu / addiu $v0,$zero,7 / bne, with the
 *           8 store constant in the delay slot). Only when that byte equals
 *           seven does it store 8 into the work byte at offset 0x9, then
 *           re-read the cell and increment the work byte at offset 0x1 by one
 *           at byte width (lbu / addiu $v0,$v0,1 / sb), which moves the stage
 *           index this address implements from 1 to 2 and so hands the next
 *           dispatch to word 2 of the run, 0x801EEEC0. Takes no arguments,
 *           returns nothing, keeps no frame and reads or writes no other state.
 *           The word at 0x801EEC08 is the only in-image reference to this
 *           address and no jal in the payload reaches it, which is the dispatch
 *           shape rather than a missing caller. No byte-identical twin exists
 *           among this tree's exact lifts, so the statement order and operand
 *           forms come from the near-identical
 *           setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBBelow5Bmagic
 *           Magic07003_801EF15C (0x801EF15C), ...Below2BmagicMagic06203
 *           (0x801EF078) and ...ZeroBmagicMagic07603 (0x801EF11C), whose bodies
 *           differ only in the test; this function writes its own equality
 *           against seven.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBIs7BmagicMagic10403_801EEE7C(
    void) {
  if (g_battle_work[0xB] == 7) {
    g_battle_work[9] = 8;
    g_battle_work[1]++;
  }
}
