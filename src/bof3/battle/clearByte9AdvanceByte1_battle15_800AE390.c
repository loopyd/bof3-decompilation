#include "bof3/battle/battle15_internal.h"

/* @source 0x800AE390
 * @behavior UNKNOWN: exact behavior is not yet documented.
 * @status exact
 * @match 100.00
 * @residual none
 */

void clearByte9AdvanceByte1(void) {
  u8* work;

  g_battle_work[9] = 0;
  work = g_battle_work;
  work[1] = (u8)(work[1] + 1);
}
