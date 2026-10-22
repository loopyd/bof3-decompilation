#include "bof3/ui/game00_internal.h"

extern s32 func_801A1C5C(s32 x, s32 y, s32 route);
extern s32 func_801A1ED8(s32 x, s32 y, s32 route);

/* @behavior projects the supplied coordinates along the active scratch route,
 * dispatches by movement speed, and returns the low byte of the result.
 * @source 0x801A1BC0
 * @status partial
 * @match 79.49
 * @residual First differing byte is +0x0034: the first route-offset product lands in
 *           $t0 here and $t1 in the original. The original keeps the projected Y word
 *           in $a1 (mflo a1), copies it through $v0 across the speed branch and sets
 *           $a0 = x once before that branch; here the local that receives the callee
 *           result also carries the projected Y word, so Y lives in $v0 and both arms
 *           copy $a0/$a1 while the Y sum fills the branch delay slot. The route mask,
 *           both route-offset products, both coordinate loads, the 24-byte frame and
 *           the 156-byte size match the original. Clean-C allocator/scheduling
 *           residual.
 */
s32 func_801A1BC0(struct GameWorkArea* arg0) {
  struct GameWorkArea* work;
  s32                  route;
  u8                   speed;
  s32                  x;
  s32                  y;

  work = g_game_work;
  route = work->route_index_08 & 7;
  speed = work->speed_70;
  x = arg0->coord_x_34;
  y = arg0->coord_y_38;
  x += D_80181B94[route * 2] * (s32)(speed + 1);
  y += D_80181B98[route * 2] * (s32)(speed + 1);
  if (speed == 0) {
    y = func_801A1ED8(x, y, route);
  } else {
    y = func_801A1C5C(x, y, route);
  }
  return y & 0xff;
}
