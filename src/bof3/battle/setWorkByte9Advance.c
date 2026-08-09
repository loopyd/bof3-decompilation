#include "bof3/battle/battle15_internal.h"

/* @behavior Updates two bytes through the scratchpad work-area pointer.
 * @source 0x800AE06C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void setWorkByte9Advance(void) {
  g_battle_work[9] = 0x3C;
  g_battle_work[1]++;
}
