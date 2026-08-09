#include "bof3/ui/game00_internal.h"

extern s32 func_801A1C5C(s32 x, s32 y, s32 route, s32 arg3);
extern s32 func_801A1ED8(s32 x, s32 y, s32 route, s32 arg3);

/* @behavior projects the supplied coordinates along the active scratch route,
 * dispatches by movement speed, and returns the low byte of the result.
 * @source 0x801A1BC0
 * @status partial
 * @match 32.50
 * @residual non-exact live audit: 13/39 instructions; 156 original bytes versus 160 current.
 */
s32 func_801A1BC0(struct GameWorkArea* arg0) {
  volatile struct GameWorkArea* work;
  u8                            speed;
  s32                           route;
  s32                           x;
  s32                           y;

  work = SCRATCH_WORK;
  speed = work->speed_70;
  route = work->route_index_08 & 7;
  x = arg0->coord_x_34 + MOVEMENT_OFFSET_0(route) * (s32)(speed + 1);
  y = arg0->coord_y_38 + MOVEMENT_OFFSET_1(route) * (s32)(speed + 1);
  if (speed == 0) {
    return func_801A1ED8(x, y, route, route) & 0xff;
  }
  return func_801A1C5C(x, y, route, route) & 0xff;
}
