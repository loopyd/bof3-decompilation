#include "bof3/battle/battle03_internal.h"

/* @behavior clears the first three bytes of all eight event-queue slots.
 * @source 0x801DE804
 * @status partial
 * @match 29.17
 * @residual non-exact live audit: 7/21 instructions; 84 original bytes versus 96 current.
 */
void func_801DE804(void) {
  u8  index;
  u32 offset;

  index = 0u;
  do {
    offset = (u32)index * 0xcu;
    BATTLE_EVENT_SLOT_FLAG(index) = 0u;
    BATTLE_EVENT_SLOT_A(index) = 0u;
    BATTLE_EVENT_SLOT_B(index) = 0u;
    index += 1u;
  } while (index < 8u);
}
