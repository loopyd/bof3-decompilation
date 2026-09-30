#include "bof3/ui/game00_internal.h"

/* @behavior Classifies a requested route index against the scratchpad work
 * area route_index_08 within a wrapped 8-entry window. The reference bound is
 * the current route index, or its next 8-wide boundary when the requested
 * index is ahead of it; the result is -1 when that bound lies 1 to 4 entries
 * above the requested index and 1 otherwise.
 * @source 0x801B2D00
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 routeIndexStepSign(u8 index) {
  u8  current;
  u8  bound;
  s32 target;

  target = index;
  current = g_game_work->route_index_08;
  bound = current;
  if ((u32)current < (u32)target) {
    bound = (current + 8) & 0xFF;
  }
  if (bound >= target + 1 && target + 4 >= bound) {
    return -1;
  }
  return 1;
}
