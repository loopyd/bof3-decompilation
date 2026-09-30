#include "bof3/ui/game00_internal.h"

/* @source 0x8019C1E4
 * @behavior projects the active work area's 16.16 x/y pair at 0x34/0x38 along
 * the movement-offset record selected by its route byte at 0x08 (offsets are
 * doubled before they are added) and hands the projected pair to
 * func_8019C344, returning the low byte of that acceptance result.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_8019C1E4(void) {
  struct GameWorkArea* work;
  s32 x;
  s32 y;

  work = g_game_work;
  x = work->coord_x_34 + (D_80181B94[work->route_index_08 * 2] << 1);
  y = work->coord_y_38 + (D_80181B98[work->route_index_08 * 2] << 1);
  return func_8019C344(x, y);
}
