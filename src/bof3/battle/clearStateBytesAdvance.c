#include "bof3/bof3.h"
#include "bof3/battle/battle03_internal.h"

/* @source 0x801D67EC
 * @behavior clears two battle-state bytes and increments one global counter.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void clearStateBytesAdvance(void) {
  D_8014864C = 0;
  D_801462E5 = 0;
  BATTLE_GLOBAL_BYTE_62E2 += 1;
}
