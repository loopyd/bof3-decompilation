#include "bof3/battle/battle03_internal.h"

/* @behavior scans up to three paired 0x140-stride work/status records, returning
 * zero at the first inactive work record or one at the first active status.
 * @source 0x801E7B34
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801E7B34(void) {
  u8 index;

  index = 0u;
  while (index < 3u) {
    if ((localWorkRecordsBf20[index].flags_00 & 1u) == 0u) {
      return 0u;
    }
    if ((battleLocalStatusArray[index].flags_00 & 1u) != 0u) {
      return 1u;
    }
    index += 1u;
  }
  return 0u;
}
