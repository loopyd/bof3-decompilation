#include "internal.h"

/* @behavior reports whether the UI ring currently contains queued work.
 * @source 0x801DE92C
 */
u8 func_801DE92C(void) {
  volatile u8* const base = (volatile u8*)0x801f0000u;
  s16                target_offset;
  s16                index_offset;
  u8                 index;
  u8                 target;

  target_offset = (s16)0xc328u;
  index_offset = (s16)0xbf04u;
  target = base[target_offset];
  index = base[index_offset];
  return index != target;
}
