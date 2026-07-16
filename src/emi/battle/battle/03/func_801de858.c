#include "internal.h"

/* @behavior reports whether any event-queue slot is active for the given event type
 * byte.
 * @source 0x801DE858
 */
u8 func_801DE858(s8 arg0) {
  u8  index;
  u32 offset;

  index = 0u;
  while (index < 8u) {
    offset = (u32)index * 0xcu;
    if ((((volatile u8*)0x801f0000u)[offset - 0x4b10u] != 0u) &&
        (((volatile u8*)0x801f0000u)[offset - 0x4b0fu] == (u8)arg0)) {
      return 0u;
    }
    index += 1u;
  }
  return 1u;
}
