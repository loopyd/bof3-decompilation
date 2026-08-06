#include "internal.h"

/* @source 0x801E6990
 * @behavior clears work byte nine and increments work byte one.
 */
void clearByte9AdvanceByte1(void) {
  u8* work;

  battleWork[9] = 0;
  work = battleWork;
  work[1] = (u8)(work[1] + 1);
}
