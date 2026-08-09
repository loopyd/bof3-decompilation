#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @initializes several global state variables.
 * @source 0x800A4AD0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_800A4AD0(void) {
  u8 value = D_801462E5;

  D_801462E4 = 0;
  D_801462E6 = 0;
  D_801462E3 = value + 3;
  D_801462E5 = value | 0x80;
}
