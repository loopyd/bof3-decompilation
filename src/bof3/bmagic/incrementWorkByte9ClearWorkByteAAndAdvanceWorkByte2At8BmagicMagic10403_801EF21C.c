#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF21C
 * @behavior Handler at index 1 of the work-byte-2 dispatch table at
 *           0x801F08F0. Increments work byte 0x9 modulo 256 and compares the
 *           value just stored against 8 without reloading it. On equality,
 *           reloads the scratchpad work-object pointer cell at 0x1F800044 to
 *           clear work byte 0xA, then reloads the cell again to increment work
 *           byte 0x2. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9ClearWorkByteAAndAdvanceWorkByte2At8BmagicMagic10403_801EF21C(void) {
  if (++g_battle_work[9] == 8) {
    g_battle_work[0xA] = 0;
    g_battle_work[2]++;
  }
}
