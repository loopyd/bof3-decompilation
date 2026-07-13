#include "internal.h"

extern s32 func_801a1c5c(s32 x, s32 y, s32 route, s32 arg3);
extern s32 func_801a1ed8(s32 x, s32 y, s32 route, s32 arg3);

/* @behavior projects the supplied coordinates along the active scratch route,
 * dispatches by movement speed, and returns the low byte of the result.
 * @source 0x801a1bc0 FUN_801a1bc0
 */
s32 func_801a1bc0(struct GameWorkArea* arg0) {
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
    return func_801a1ed8(x, y, route, route) & 0xff;
  }
  return func_801a1c5c(x, y, route, route) & 0xff;
}
