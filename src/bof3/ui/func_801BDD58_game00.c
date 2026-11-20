#include "bof3/ui/game00_internal.h"
#include <stdlib.h>

/**
 * @source 0x801BDD58
 * @behavior Returns 1 when the absolute difference between signed counter_3E
 * and the signed low halfword of reference is at most
 * (range + speed_70) * 128 + 256 and both projected coordinate differences
 * have absolute values below (range + speed_70 + 2) * 32768. The projections
 * add field_0C * pad_09[0] to coord_x_34 and field_10 * pad_09[0] to coord_y_38.
 * Returns 0 when a gate fails.
 * @status partial
 * @match 95.74
 * @residual 45/47 instructions at the original 188 bytes; first difference
 * +0x84. The Y-product retrieval and sum use V0 instead of T2.
 */
u8 func_801BDD58(s32 x, s32 y, s32 reference, s32 range,
                 struct GameWorkArea* work) {
  s32 projected_x;
  s32 projected_y;
  s32 y_product;
  s32 origin_y;
  s32 step;
  s32 distance;
  union {
    struct GameWorkArea* work;
    s32 extent;
  } phase;
  u8 matches;

  phase.work = work;
  projected_y = (s16)reference;
  distance = (s16)phase.work->counter_3E;
  distance -= projected_y;
  distance = abs(distance);
  if (distance > (range + phase.work->speed_70) * 128 + 256) {
    return 0;
  }
  do {
    step = phase.work->pad_09[0];
    projected_y = phase.work->field_0C * step;
    do {
      y_product = phase.work->field_10;
      y_product *= step;
      y_product = (s32)((u32)y_product - 1U);
      y_product = (s32)~(u32)y_product;
      projected_x = phase.work->coord_x_34 + projected_y;
      origin_y = phase.work->coord_y_38;
      origin_y = (s32)((u32)origin_y - (u32)y_product);
      distance = x - projected_x;
      distance = abs(distance);
      projected_y = origin_y;
    } while (0);
    phase.extent = range + phase.work->speed_70 + 2;
    phase.extent = (s32)((u32)phase.extent << 15);
    distance = distance < phase.extent;
  } while (0);
  matches = 0;
  if (distance) {
    matches = abs(y - projected_y) < phase.extent;
  }
  return matches;
}
