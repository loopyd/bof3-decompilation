#include "bof3/world/area03004_internal.h"

/* @behavior clears the scratch-record flag when the AREA030 state byte is >= 10.
 * @source 0x801E023C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void clearFlagState10(void) {
  if (D_80145E93 >= 10u) {
    *WORLD00_AREA030_SCRATCH_PTR = 0u;
  }
}
