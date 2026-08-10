#include "bof3/ui/game00_internal.h"

/* @behavior clears all nine bytes in the three-by-three state table.
 * @source 0x801C1840
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801C1840(void) {
  s32 outer;
  s32 inner;

  for (outer = 0; outer < 3; outer++) {
    for (inner = 2; inner >= 0; inner--) {
      D_801454F4[outer][inner] = 0;
    }
  }
}
