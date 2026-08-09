#include "bof3/battle/battle03_internal.h"

/* @source 0x801E6990
 * @behavior clears work byte nine and increments work byte one.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void clearByte9AdvanceByte1(void) {
  u8* work;

  battleWork[9] = 0;
  work = battleWork;
  work[1] = (u8)(work[1] + 1);
}
