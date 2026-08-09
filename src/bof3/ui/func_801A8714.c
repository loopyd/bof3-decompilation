#include "bof3/ui/game00_internal.h"

/* @behavior stores 0xF0 to D_80146864 and returns zero.
 * @source 0x801A8714
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 func_801A8714(void) {
  D_80146864 = 0xF0;
  return 0;
}
