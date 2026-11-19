#include "bof3/ui/game00_internal.h"
#include <stdlib.h>

/**
 * @source 0x801BDD58
 * @behavior Returns 1 when the absolute difference between signed counter_3E
 * and reference is at most
 * (range + speed_70) * 128 + 256 and both projected coordinate differences
 * have absolute values below (range + speed_70 + 2) * 32768. The projections
 * add field_0C * pad_09[0] to coord_x_34 and field_10 * pad_09[0] to coord_y_38.
 * Returns 0 when a gate fails.
 * @status partial
 * @match 63.83
 * @residual The rebuilt image is 184 bytes versus 188 original bytes.
 * Guarded returns, multiply operands and projection/bound setup still differ.
 */
u8 func_801BDD58(s32 x, s32 y, s16 reference, s32 range,
                 struct GameWorkArea* work) {
  s32 projected_x;
  s32 projected_y;
  s32 distance;

  distance = (s16)work->counter_3E;
  distance -= reference;
  distance = abs(distance);
  if (distance > (range + work->speed_70) * 128 + 256) {
    return 0;
  }
  projected_x = work->pad_09[0];
  projected_y = projected_x;
  projected_x = work->field_0C * projected_x;
  projected_y = work->field_10 * projected_y;
  projected_x += work->coord_x_34;
  projected_y += work->coord_y_38;
  distance = x - projected_x;
  distance = abs(distance);
  distance = distance < (range + work->speed_70 + 2) * 32768;
  if (!distance) {
    return distance;
  }
  return abs(y - projected_y) < (range + work->speed_70 + 2) * 32768;
}
