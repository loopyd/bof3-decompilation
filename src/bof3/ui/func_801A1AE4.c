#include "bof3/ui/game00_internal.h"

extern s32 func_801A1BC0(struct GameWorkArea* arg);
extern s8 func_801BDE14(s32 x, s32 y, s32 range);
extern s32 func_801BE0C0(s32 arg0, s32 arg1, u8 arg2);

/* @behavior checks movement to a position determined by the route index,
 *        using two offset tables.
 * @source 0x801A1AE4
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801A1AE4(struct GameWorkArea* arg) {
  s32 s0;
  s32 s1;
  s32 second_x;
  s32 second_y;
  s32 result;
  volatile struct GameWorkArea* work;
  u8  route;

  route = arg->route_index_08 & 7;
  s0 = arg->coord_x_34 + D_80181B94[route * 2];
  s1 = arg->coord_y_38 + D_80181B98[route * 2];

  result = func_801A1BC0(arg);
  second_x = s0;
  second_y = s1;
  if (result & 0xFF) {
    goto success;
  }
  work = g_game_work;
  s0 = func_801BDE14(s0, s1, work->speed_70);
  if ((s8)s0 == -1) {
    goto final_move;
  }
success:
  return 1;
final_move:
  work = g_game_work;
  s1 = work->speed_70;
  s0 = func_801BE0C0(second_x, second_y, (u8)s1);
  s1 = (s8)s0 != -1;
  return s1;
}
