#include "internal.h"

/* @source 0x801E6990
 * @behavior clears work byte nine and increments work byte one.
 */
void func_801E6990(void) {
  u8* work;

  g_battle03_work[9] = 0;
  work = g_battle03_work;
  work[1] = (u8)(work[1] + 1);
}
