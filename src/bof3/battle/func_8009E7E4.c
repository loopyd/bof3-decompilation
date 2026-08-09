#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @calls func_800A403C with argument 2
 * @source 0x8009E7E4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8009E7E4(void) {
  func_800A403C(2);
}
