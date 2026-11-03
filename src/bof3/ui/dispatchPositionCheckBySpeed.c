#include "bof3/ui/game00_internal.h"

/*
 * @source 0x801C3E40
 * @behavior Forwards the position check of the supplied x/y coordinates and
 * signed 16-bit reference to the sibling routine selected by the record speed
 * byte: a nonzero speed byte calls isPositionRejectedForNonzeroSelector and a zero speed byte calls
 * isPositionRejectedForZeroSelector, and the byte result of that routine is returned. The callers
 * pass the work record coord_x_34 / coord_y_38 words, the work record speed_70
 * byte and the work record counter_3E halfword.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 dispatchPositionCheckBySpeed(s32 x, s32 y, u8 speed, s16 reference)
{
  u8 result;

  if (speed != 0) {
    result = isPositionRejectedForNonzeroSelector(x, y, reference);
  } else {
    result = isPositionRejectedForZeroSelector(x, y, reference);
  }
  return result;
}
