#include "bof3/battle/battle03_internal.h"

/* @behavior Returns the index with the smallest halfword at +0x88 among the
 * first three D_80145E90 records not blocked by isBattlerInactiveOrFlag4000. Only values below
 * 10000 qualify; ties keep the earlier index, and no qualifying value returns 0.
 * @source 0x801E2D90
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801E2D90(void) {
  u32 index;
  u32 best;
  u32 result;
  u8  slot;
  u16 value;

  result = 0u;
  best = 10000u;
  index = 0u;
  do {
    slot = (u8)index;
    if (isBattlerInactiveOrFlag4000(slot) == 0u) {
      value = *(u16*)&D_80145F18[((slot << 2) + slot) << 6];
      if (value < best) {
        best = value;
        result = index;
      }
    }
    index += 1u;
  } while ((u8)index < 3u);

  return (u8)result;
}
