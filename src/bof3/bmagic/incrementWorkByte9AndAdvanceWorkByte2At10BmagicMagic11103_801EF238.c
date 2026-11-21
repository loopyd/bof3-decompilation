#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF238
 * @behavior Adds one modulo 256 to unsigned byte 9 of the work object
 *           reached through the scratchpad pointer cell at 0x1F800044.
 *           When the stored byte equals 0x10, re-reads that pointer cell
 *           and increments the object's byte 2 modulo 256. Takes no
 *           arguments, returns nothing and keeps no frame. The entry is
 *           word 4 of the ten-pointer handler table based at 0x801EFA9C.
 *           Byte roles remain offset-anchored. The 80 original bytes differ
 *           from this target's 0x801EF8C4 handler in one word only: the
 *           addiu immediate, which adds 1 here and 2 there.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceWorkByte2At10BmagicMagic11103_801EF238(void) {
  if ((g_battle_work[9] += 1) == 0x10) {
    g_battle_work[2]++;
  }
}
