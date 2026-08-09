#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @source 0x80096AB0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_80096AB0(void) {
  func_801DE94C(0, 0);
  PSX_REF(u8, (u32)&D_801462E3) += 1;
}
