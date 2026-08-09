#include "bof3/battle/battle15_internal.h"

/* @behavior clears scratchpad work area bytes at offsets 0x00-0x04.
 * @source 0x800ACFF4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_800ACFF4(void) {
  g_battle_work[0] = 0;
  g_battle_work[1] = 0;
  g_battle_work[2] = 0;
  g_battle_work[3] = 0;
  g_battle_work[4] = 0;
}
