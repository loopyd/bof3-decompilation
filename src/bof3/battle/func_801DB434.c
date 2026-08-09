#include "bof3/battle/battle03_internal.h"

/* @behavior scans one six-entry threshold row and returns the first index whose
 * value exceeds the input byte, or `6` if none match.
 * @source 0x801DB434
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u32 func_801DB434(u8 arg0, u32 arg1) {
  u8 index;

  index = 0u;
  while (index < 6u) {
    if (arg0 < D_801EAF88[arg1 & 0xffu][index]) {
      return index;
    }
    index += 1u;
  }
  return 6u;
}
