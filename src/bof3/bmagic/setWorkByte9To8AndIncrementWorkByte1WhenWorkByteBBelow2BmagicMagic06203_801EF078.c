#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original re-materializes and reloads the cell
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)) around the arm's stores, so the cell is
 * reached through this target-map symbol rather than through a constant
 * address, which the compiler materializes once and reuses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF078
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 and
 *           compares the work object's byte at offset 0xB against two with an
 *           unsigned bound (lbu/sltiu), so the guard is taken for every value
 *           below two. Only on that path it stores 8 into the work byte at
 *           offset 0x9, then re-reads the cell and increments the work byte at
 *           offset 0x1 by one at byte width (lbu/addiu/sb). Word 3 of the
 *           overlay entry run at 0x801EEC04 names this address. Bytes
 *           0x801EF078..0x801EF0BF are byte-identical to the unlifted
 *           0x801EF0C4 of bmagic/magic082/03. Takes no arguments, returns
 *           nothing and keeps no frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBBelow2BmagicMagic06203_801EF078(
    void) {
  if (g_battle_work[0xB] < 2u) {
    g_battle_work[9] = 8;
    g_battle_work[1]++;
  }
}
