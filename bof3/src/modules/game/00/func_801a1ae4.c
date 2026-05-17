#include "internal.h"

s32 func_801a1bc0(void);
s32 func_801bde14(s32 arg0, s32 arg1, u8 arg2);
s32 func_801be0c0(s32 arg0, s32 arg1, u8 arg2);

/* does: checks movement to a position determined by the route index,
 *        using two offset tables.
 * @source: 0x801a1ae4 FUN_801a1ae4
 */
s32 func_801a1ae4(struct GameWorkArea* arg) {
  s32 s0;
  s32 s1;
  u8 route;

  route = arg->route_index_08 & 7;
  s0 = arg->coord_x_34 + MOVEMENT_OFFSET_0(route);
  s1 = arg->coord_y_38 + MOVEMENT_OFFSET_1(route);

  if (func_801a1bc0() & 0xFF) {
    return 1;
  }

  if ((s8)func_801bde14(s0, s1, SCRATCH_WORK->speed_70) != -1) {
    return 1;
  }

  return (s8)func_801be0c0(s0, s1, SCRATCH_WORK->speed_70) != -1;
}
