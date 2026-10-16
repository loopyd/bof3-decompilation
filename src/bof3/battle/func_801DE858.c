#include "bof3/battle/battle03_internal.h"

/* @behavior returns zero if any of eight event-queue slots has a nonzero flag
 * byte and a type matching the argument low byte, or one otherwise.
 * @source 0x801DE858
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801DE858(s8 arg0) {
  u8 index;
  u32 offset;

  index = 0u;
  while (index < 8u) {
    offset = (u32)index * 0xcu;
    if ((battleDispatchSlots[offset] != 0u) &&
        (battleDispatchSlots[offset + 1u] == (u8)arg0)) {
      return 0u;
    }
    index += 1u;
  }
  return 1u;
}
