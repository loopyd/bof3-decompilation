#include "bof3/battle/battle03_internal.h"

/* @behavior reports whether any event-queue slot is active for the given event type
 * byte.
 * @source 0x801DE858
 * @status partial
 * @match 40.74
 * @residual non-exact live audit: 11/26 instructions; 104 original bytes versus 108 current.
 */
u8 func_801DE858(s8 arg0) {
  u8 index;

  index = 0u;
  while (index < 8u) {
    if ((BATTLE_EVENT_SLOT_FLAG(index) != 0u) &&
        (BATTLE_EVENT_SLOT_A(index) == (u8)arg0)) {
      return 0u;
    }
    index += 1u;
  }
  return 1u;
}
