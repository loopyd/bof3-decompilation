#include "internal.h"

/* @behavior reports whether the UI ring currently contains queued work.
 * @source 0x801DE92C
 */
u8 hasUiRingWork(void) {
  s16 target_offset;
  s16 index_offset;
  u8  index;
  u8  target;

  target_offset = (s16)0xc328u;
  index_offset = (s16)0xbf04u;
  target = BATTLE_HIGH_RAM_U8[target_offset];
  index = BATTLE_HIGH_RAM_U8[index_offset];
  return index != target;
}
