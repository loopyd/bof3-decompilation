#include "internal.h"

/* does: ticks an animation-position counter, computes offset-adjusted
 *        coordinates, and sets a completion flag when the counter crosses a
 *        per-entity threshold.
 * @source: 0x801c5558 FUN_801c5558
 */
void func_801c5558(void) {
  struct GameWorkArea* work;
  u32                  route;
  u32                  offs_a;
  u32                  offs_b;
  s16                  result;
  u8*                  global_work;
  s16                  threshold;

  work = SCRATCH_WORK;
  route = work->route_index_08 * 8;

  work->counter_3E += 16;

  offs_a = MOVEMENT_OFFSET_0(route) * (u32)(work->speed_70 + 2);
  offs_b = MOVEMENT_OFFSET_1(route) * (u32)(work->speed_70 + 2);

  result = func_80154f28(work->coord_x_34 + offs_a, work->coord_y_38 + offs_b);

  global_work = GLOBAL_WORK_PTR;
  threshold = MOVEMENT_THRESHOLD(global_work[0x79]);

  work = SCRATCH_WORK;

  if ((s16)work->counter_3E >= result - threshold) {
    work->flags_02 = 3;
  }

  func_8014d978();
}
