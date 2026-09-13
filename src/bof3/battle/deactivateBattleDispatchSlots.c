#include "bof3/battle/battle03_internal.h"

/* @behavior clears the first three bytes of all eight event-queue slots.
 * @source 0x801DE804
 * @status exact
 * @match 100.00
 * @residual none
 */
void deactivateBattleDispatchSlots(void) {
  u8  index;
  u32 offset;

  index = 0u;
  do {
    offset = (u32)index * 0xcu;
    battleDispatchSlots[offset] = 0u;
    battleDispatchSlots[offset + 1u] = 0u;
    battleDispatchSlots[offset + 2u] = 0u;
    index += 1u;
  } while (index < 8u);
}
