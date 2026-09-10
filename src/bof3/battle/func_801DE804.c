#include "bof3/battle/battle03_internal.h"

/* @behavior clears the first three bytes of all eight event-queue slots.
 * @source 0x801DE804
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DE804(void) {
  u8  index;
  u32 offset;

  index = 0u;
  do {
    offset = (u32)index * 0xcu;
    D_801EB4F0[offset] = 0u;
    D_801EB4F0[offset + 1u] = 0u;
    D_801EB4F0[offset + 2u] = 0u;
    index += 1u;
  } while (index < 8u);
}
