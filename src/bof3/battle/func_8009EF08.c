#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @calls func_800A403C with argument 3
 * @source 0x8009EF08
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8009EF08(void) {
  func_800A403C(3);
}
