#include "bof3/ui/game00_internal.h"

extern s32 func_801A1BC0(void);
extern s32 func_801BDE14(s32 arg0, s32 arg1, u8 arg2);
extern s32 func_801BE0C0(s32 arg0, s32 arg1, u8 arg2);

/* @behavior checks movement to a position determined by the route index,
 *        using two offset tables.
 * @source 0x801A1AE4
 * @status partial
 * @match 45.45
 * @residual non-exact live audit: 25/55 instructions; 220 original bytes versus 216 current.
 */
s32 func_801A1AE4(struct GameWorkArea* arg) {
  s32 s0;
  s32 s1;
  u8  route;

  route = arg->route_index_08 & 7;
  s0 = arg->coord_x_34 + MOVEMENT_OFFSET_0(route);
  s1 = arg->coord_y_38 + MOVEMENT_OFFSET_1(route);

  if (func_801A1BC0() & 0xFF) {
    return 1;
  }

  if ((s8)func_801BDE14(s0, s1, SCRATCH_WORK->speed_70) != -1) {
    return 1;
  }

  return (s8)func_801BE0C0(s0, s1, SCRATCH_WORK->speed_70) != -1;
}
