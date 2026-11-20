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
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 matchesProjectedWorkBounds(s32 x, s32 y, s32 reference, s32 range,
                 struct GameWorkArea* work) {
  s32 projected_y;
  s32 y_product;
  s32 step;
  s32 height_delta;
  s32 speed;
  s32 adjusted_range;
  s32 height_limit;
  s32 outside_height;
  s32 distance;
  u8 matches;
  union {
    struct GameWorkArea* work;
    s32 origin_y;
    s32 extent;
  } phase;

  projected_y = (s16)reference;
  do {
    do {
      distance = (s16)work->counter_3E;
    } while (0);
    speed = work->speed_70;
    height_delta = distance - projected_y;
    speed = -speed;
    height_delta = abs(height_delta);
    adjusted_range = range - speed;
    height_limit = adjusted_range * 128 + 256;
    do {
      outside_height = height_delta > height_limit;
    } while (0);
    phase.work = work;
    if (outside_height) {
      matches = 0;
    miss:
      return matches;
    }
  } while (0);
  do {
    step = phase.work->pad_09[0];
    projected_y = phase.work->field_0C * step;
    do {
      y_product = phase.work->field_10 * step;
      distance = phase.work->coord_x_34;
      phase.origin_y = phase.work->coord_y_38;
      distance += projected_y;
      distance = x - distance;
      distance = abs(distance);
      projected_y = phase.origin_y + y_product;
    } while (0);
    phase.extent = adjusted_range + 2;
    phase.extent = (s32)((u32)phase.extent << 15);
    distance = distance < phase.extent;
  } while (0);
  matches = 0;
  if (!distance) {
    goto miss;
  }
  matches = abs(y - projected_y) < phase.extent;
  return matches;
}
