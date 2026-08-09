#include "bof3/world/area02713_internal.h"

/**
 * @source 0x801F35E0
 * @behavior Resets a state value and conditionally enables a pair of flags.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801F35E0(void) {
  WORLD00_AREA027_STATE_90A8 = 0xFFFF;
  if (!(WORLD00_AREA027_FLAGS_90C7 & 1)) {
    WORLD00_AREA027_FLAG_48EB = 1;
    WORLD00_AREA027_FLAG_48EC = 0;
  }
}
