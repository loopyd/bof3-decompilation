#include "bof3/battle/battle15_internal.h"

/* @calls func_800A403C with argument 0
 * @source 0x8009E1E0
 * @behavior forwards selector 0 to func_800A403C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void forwardSelector0(void) {
  func_800A403C(0);
}
