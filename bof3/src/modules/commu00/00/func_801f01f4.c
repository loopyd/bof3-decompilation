#include "internal.h"

/* does: finds one active COMMU00 source slot from a random start, prefers not
 * to clear kind-9 rows until one full wrap, then appends the removed slot id
 * to the removal queue.
 * @source: 0x801f01f4 FUN_801f01f4
 */
void func_801f01f4(void) {
  s32 active_offset;
  u8  start_index;
  u8  source_index;
  u8  allow_type9_clear;

  allow_type9_clear = 0u;
  source_index = (u8)(game_random_u16() & 0x3fu);
  if (source_index > 0x3bu) {
    source_index = (u8)(source_index - 0x3cu);
  }

  start_index = source_index;
  while (true) {
    active_offset = (s32)((u32)source_index * 8u);
    if (((const volatile u8*)0x801455c8u)[active_offset] != 0u) {
      if ((((const volatile u8*)0x801455c9u)[active_offset] != 9u) ||
          (allow_type9_clear != 0u)) {
        ((volatile u8*)0x801455c8u)[active_offset] = 0u;
        ((volatile u8*)0x80145e30u)[(*(volatile u8*)0x80145e44u)] =
            source_index;
        (*(volatile u8*)0x80145e44u) += 1u;
        return;
      }
    }

    source_index += 1u;
    if (source_index > 0x3bu) {
      source_index = 0u;
    }
    if (source_index == start_index) {
      allow_type9_clear = 1u;
    }
  }
}
