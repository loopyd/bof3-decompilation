#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The original keeps the
 * base it loaded for the guard through the 0x9 store, then reloads the cell
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)) for the 0x1 increment because the store
 * through the base may alias the cell itself, so the cell is reached through
 * this symbol rather than through a constant address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEF98
 * @behavior Third entry of the five-word handler run at 0x801EEC04, selected
 *           by work byte 1 through the dispatcher at 0x801EEC18, which copies
 *           the run into its frame and calls the word at that index. Reads the
 *           scratchpad work-object pointer cell at 0x1F800044 and tests the
 *           work object's byte at offset 0xB against zero with a plain branch
 *           on the loaded byte (lbu / beqz), so the arm runs only when that
 *           byte is non-zero. In the arm it stores 8 into the work byte at
 *           offset 0x9, then re-reads the cell and increments the work byte at
 *           offset 0x1 by one at byte width (lbu / addiu $v0,$v0,1 / sb), which
 *           moves the stage index this address implements from 2 to 3 and so
 *           hands the next dispatch to word 3 of the run, 0x801EEFDC. The 8
 *           store constant fills the test's branch delay slot. Takes no
 *           arguments, returns nothing, keeps no frame and reads or writes no
 *           other state. The word at 0x801EEC0C is the only reference to this
 *           address in the payload and no jal reaches it, which is the dispatch
 *           shape rather than a missing caller. One word of the 0x44-byte span
 *           differs from the exact sibling
 *           setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBZeroBmagicMagic07603_801EF11C
 *           at 0x51C of its own overlay (0x14400009 bnez there, 0x10400009
 *           beqz here), so this body took that sibling's statement order,
 *           operand forms, offset spelling and declarations and wrote its own
 *           test direction.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBNonzeroBmagicMagic09903_801EEF98(
    void) {
  if (g_battle_work[0xB] != 0) {
    g_battle_work[9] = 8;
    g_battle_work[1]++;
  }
}
