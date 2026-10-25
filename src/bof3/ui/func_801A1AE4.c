#include "bof3/ui/game00_internal.h"

extern s32 func_801A1BC0(struct GameWorkArea* arg);
extern s8 func_801BDE14(s32 x, s32 y, s32 range);
extern s32 func_801BE0C0(s32 arg0, s32 arg1, u8 arg2);

/* @behavior checks movement to a position determined by the route index,
 *        using two offset tables.
 * @source 0x801A1AE4
 * @status partial
 * @match 70.91
 * @residual non-exact live comparison: 39/55 instructions; 220 original bytes versus 200 current.
 */
s32 func_801A1AE4(struct GameWorkArea* arg) {
  s32 s0;
  s32 s1;
  volatile struct GameWorkArea* work;
  u8  route;

  route = arg->route_index_08 & 7;
  s0 = arg->coord_x_34 + D_80181B94[route * 2];
  s1 = arg->coord_y_38 + D_80181B98[route * 2];

  if (!(func_801A1BC0(arg) & 0xFF)) {
    work = g_game_work;
    if ((s8)func_801BDE14(s0, s1, work->speed_70) == -1) {
      work = g_game_work;
      return (s8)func_801BE0C0(s0, s1, work->speed_70) != -1;
    }
  }
  return 1;
}
