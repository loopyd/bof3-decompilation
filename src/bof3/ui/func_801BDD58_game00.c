#include "bof3/ui/game00_internal.h"
#include <stdlib.h>

/**
 * @source 0x801BDD58
 * @behavior Returns 1 when the supplied (x, y) lies within the work record's
 * range: the absolute difference between the record counter at 0x3E and the
 * signed reference must be below (range + record speed byte 0x70) * 128 + 256,
 * the absolute difference between x and the record's projected X
 * (coord_x_34 + field_0C * pad_09[0]) must be below (range + speed + 2) * 32768,
 * and the result reports whether the absolute difference between y and the
 * projected Y (coord_y_38 + field_10 * pad_09[0]) is below that same limit; any
 * failed gate returns 0.
 * @status partial
 * @match 40.43
 * @residual allocator-only residual: the rebuilt 45-instruction sequence,
 * operand shapes and statement order match the 47-instruction original, but the
 * original copies x out of $a0 (move t0,a0) and loads the work pointer into
 * $a0, while this build keeps x in $a0 and loads work into $t0, and the two
 * early return-0 blocks are then laid out differently (inline branch-around
 * versus branch to the shared early return). Local declaration order, separate
 * abs temporaries, an explicit conditional negate, the success-path guard form,
 * projection locals and inline-versus-local limits all compile to the identical
 * 45-instruction object, so the gap is an allocation/scheduling class rather
 * than a clean-C shape; the smallest missing evidence is a per-object compiler
 * profile (opt-in rung, unauthorized for this mission).
 * Live audit: 19/47 instructions, 180 current bytes versus 188 original.
 */
u8 func_801BDD58(s32 x, s32 y, s16 reference, s32 range,
                 struct GameWorkArea* work) {
  s32 projected_x;
  s32 projected_y;

  if (abs((s16)work->counter_3E - reference) >
      (range + work->speed_70) * 128 + 256) {
    return 0;
  }
  projected_x = work->coord_x_34 + work->field_0C * work->pad_09[0];
  projected_y = work->coord_y_38 + work->field_10 * work->pad_09[0];
  if (abs(x - projected_x) >= (range + work->speed_70 + 2) * 32768) {
    return 0;
  }
  return abs(y - projected_y) < (range + work->speed_70 + 2) * 32768;
}
