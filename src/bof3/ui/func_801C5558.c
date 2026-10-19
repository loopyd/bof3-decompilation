#include "bof3/ui/game00_internal.h"

/* @behavior ticks an animation-position counter, computes offset-adjusted
 *        coordinates, and sets a completion flag when the counter crosses a
 *        per-entity threshold.
 * @source 0x801C5558
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801C5558(void) {
  volatile struct GameWorkArea* work;
  u32 route;
  u16 counter;
  s32 speed;
  s32 offs_a;
  s32 offs_b;
  s16 result;
  s16 threshold;
  u8 threshold_index;

  work = g_game_work;
  counter = work->counter_3E;
  route = work->route_index_08;
  work->counter_3E = counter + 16;
  speed = work->speed_70 + 2;
  offs_a = D_80181B94[route * 2] * speed;
  offs_b = D_80181B98[route * 2] * speed;
  result = func_80154F28(work->coord_x_34 + offs_a, work->coord_y_38 + offs_b);
  threshold_index = D_80146250[0x79];
  work = g_game_work;
  threshold = movementThresholds[threshold_index];
  if (FIELD_REF(s16, work, 0x3E) >= result - threshold) {
    work->flags_02 = 3;
  }
  func_8014D978();
}
