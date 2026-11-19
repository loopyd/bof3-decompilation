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
 * @match 78.72
 * @residual The rebuilt image is 188 bytes, matching the original size.
 * Multiply operands, product registers and bound setup still differ.
 */
u8 func_801BDD58(s32 x, s32 y, s16 reference, s32 range,
                 struct GameWorkArea* work) {
  s32 projected_x;
  s32 projected_y;
  s32 x_coefficient;
  s32 distance;
  s32 bound;
  u8 matches;

  distance = (s16)work->counter_3E;
  distance -= reference;
  distance = abs(distance);
  if (distance > (range + work->speed_70) * 128 + 256) {
    return 0;
  }
  do {
    projected_x = work->pad_09[0];
    x_coefficient = work->field_0C;
    projected_y = projected_x;
    projected_x = x_coefficient * projected_x;
    projected_y = work->field_10 * projected_y;
    projected_x += work->coord_x_34;
    projected_y += work->coord_y_38;
    distance = x - projected_x;
    distance = abs(distance);
    bound = (range + work->speed_70 + 2) * 32768;
    distance = distance < bound;
  } while (0);
  matches = 0;
  if (distance) {
    matches = abs(y - projected_y) < bound;
  }
  return matches;
}
