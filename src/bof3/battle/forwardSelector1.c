#include "bof3/battle/battle15_internal.h"

/* @calls func_800A403C with argument 1
 * @source 0x8009E7C4
 * @behavior forwards selector 1 to func_800A403C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void forwardSelector1(void) {
  func_800A403C(1);
}
