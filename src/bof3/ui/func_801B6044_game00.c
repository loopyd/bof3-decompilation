#include "bof3/ui/game00_internal.h"

/* @source 0x801B6044
 * @behavior requires selection record byte 0x79 to be 2 and work-area route bit
 *           0 to be set, then increments work byte 0x0B once per call; on the
 *           call that reaches 16 it sets work byte 0x01 to 10, clears work byte
 *           0x02 and sets selection record byte 0x12B to 8, returning 1 only on
 *           that transition.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801B6044(void) {
  if (D_80146250[0x79] != 2) {
    return 0;
  }
  if ((g_game_work->route_index_08 & 0x1) == 0) {
    return 0;
  }
  g_game_work->field_0B++;
  if (g_game_work->field_0B != 16) {
    return 0;
  }
  g_game_work->unk_01 = 10;
  g_game_work->flags_02 = 0;
  D_80146250[0x12B] = 8;
  return 1;
}
