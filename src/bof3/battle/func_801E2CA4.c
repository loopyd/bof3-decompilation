#include "bof3/battle/battle03_internal.h"

/* @behavior scans ids `3..10` and returns the unblocked id with the smallest enemy
 * halfword at offset `0x94`.
 * @source 0x801E2CA4
 * @status exact
 * @match 100.00
 * @residual none
 * The u32 counter with per-use `(u8)` casts keeps the original's per-iteration
 * zero-extension of the call argument, the record index and the loop test.
 */
u8 func_801E2CA4(void) {
  u32 index;
  u32 best;
  u8  result;
  u16 value;

  result = 0u;
  best = 0xffffu;
  index = 3u;
  do {
    if (isBattlerInactiveOrFlag4000((u8)index) == 0u) {
      value = D_801EB630[(u8)index - 3u].unk_94;
      if (value < best) {
        best = value;
        result = index;
      }
    }
    index += 1u;
  } while ((u8)index < 0x0bu);

  return result;
}
