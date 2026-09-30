#include "bof3/ui/game00_internal.h"
#include <stdlib.h>

/* @behavior projects the work record's 16.16 coordinates along the movement
 * offset pair selected by its route index (byte 0x08 masked with 7) and reports
 * whether the projected position lies inside the record's axis ranges. The X
 * axis gates the check: the result is 0 when the record X range at 0x8C is
 * smaller than the absolute delta between the projected X and the record X
 * reference at 0x82; otherwise the result reports whether the Y range at 0x8E
 * is not smaller than the absolute delta between the projected Y and the Y
 * reference at 0x86.
 * @source 0x801A1A58
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 projectedPositionWithinRange(struct GameWorkArea* work) {
  s32 x;
  s32 y;
  s32 dx;
  s32 dy;
  u8  route;

  route = work->route_index_08 & 7;
  x = work->coord_x_34 + D_80181B94[route * 2];
  y = work->coord_y_38 + D_80181B98[route * 2];
  dx = abs((x >> 16) - work->unk_82);
  if (work->unk_8C >= dx) {
    dy = abs((y >> 16) - work->unk_86);
    return !(work->unk_8E < dy);
  }
  return 0;
}
