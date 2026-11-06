#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original reaches the cell through a map symbol
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)) rather than through a constant
 * address, and it keeps that one base for the guard and the increment. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF048
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           and compares the work object's byte at offset 0xB against two with
 *           an unsigned bound (lbu/sltiu), so the guard is taken for every
 *           value below two. Only on that path it increments the work byte at
 *           offset 0x1 by one at byte width (lbu/addiu/sb) through the same
 *           base, with no second cell load. The pointer word at 0x801EEC10,
 *           the fourth entry of the overlay's leading run, names this address.
 *           Bytes 0x801EF048..0x801EF07F are byte-identical to the exact
 *           0x801EF070 of bmagic/magic088/03 and they match the shape of the
 *           exact sibling
 *           setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBBelow2BmagicMagic06203_801EF078
 *           without its store of 8 into the work byte at offset 0x9. Takes no
 *           arguments, returns nothing and keeps no frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1WhenWorkByteBBelow2BmagicMagic16703_801EF048(void) {
  if (g_battle_work[0xB] < 2u) {
    g_battle_work[1]++;
  }
}
