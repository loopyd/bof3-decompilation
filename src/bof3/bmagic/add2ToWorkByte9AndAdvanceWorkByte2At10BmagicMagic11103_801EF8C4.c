#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF8C4
 * @behavior Adds two modulo 256 to unsigned byte 9 of the work object
 *           reached through the scratchpad pointer cell at 0x1F800044.
 *           When the stored byte equals 0x10, re-reads that pointer cell
 *           and increments the object's byte 2 modulo 256. Takes no
 *           arguments, returns nothing and keeps no frame. The entry is
 *           handler 8 of the table at 0x801EFA9C, indexed by work byte 1.
 *           Byte roles remain offset-anchored. The 80 original bytes
 *           compared equal to emi/bmagic/magic041/03 at 0x801EF4D4;
 *           this body preserves that seed's named-cell access form.
 * @status exact
 * @match 100.00
 * @residual none
 */
void add2ToWorkByte9AndAdvanceWorkByte2At10BmagicMagic11103_801EF8C4(void) {
  if ((g_battle_work[9] += 2) == 0x10) {
    g_battle_work[2]++;
  }
}
