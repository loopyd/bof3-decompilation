#include "bof3/ui/game00_internal.h"

/*
 * @source 0x801C3E40
 * @behavior Forwards the position check of the supplied x/y coordinates and
 * signed 16-bit reference to the sibling routine selected by the record speed
 * byte: a nonzero speed byte calls func_801C41D0 and a zero speed byte calls
 * func_801C4040, and the byte result of that routine is returned. The callers
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
    result = func_801C41D0(x, y, reference);
  } else {
    result = func_801C4040(x, y, reference);
  }
  return result;
}
