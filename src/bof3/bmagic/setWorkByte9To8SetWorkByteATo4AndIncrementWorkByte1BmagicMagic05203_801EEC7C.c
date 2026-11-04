#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original re-materializes and reloads the cell for
 * every access (lui $v1,0x1F80 / lw $v1,0x44($v1) three times), so the cell is
 * reached through this target-map symbol rather than through a constant
 * address, which the compiler would materialize once and reuse. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEC7C
 * @behavior Arms two bytes of the scratchpad work object and advances its
 * state byte: the work-object byte at offset 0x9 is set to 8 and the byte at
 * offset 0xA is set to 4 (the original materializes each literal with
 * addiu $v0,$zero,K and stores it with sb), then the byte at offset 0x1 is
 * incremented by one at byte width (lbu, addiu $v0,$v0,1, sb scheduled into
 * the $ra delay slot). The work-object pointer cell at 0x1F800044 is
 * re-materialized for every access, so the original keeps three separate
 * lui $v1,0x1F80 / lw $v1,0x44($v1) pairs around the two stores. Nothing is
 * returned, no other state is read or written and the function has no frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To8SetWorkByteATo4AndIncrementWorkByte1BmagicMagic05203_801EEC7C(
    void) {
  g_battle_work[9] = 8;
  g_battle_work[0xA] = 4;
  g_battle_work[1]++;
}
