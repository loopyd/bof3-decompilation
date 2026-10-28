#include "bof3/ui/game00_internal.h"

/* @behavior Queries func_80154F78 with the two incoming coordinates and the scratch
 * work route index. Returns 1 only when the scratch flag at D_1F800000[0] is
 * nonzero and the signed 16-bit sample difference returned by that helper
 * exceeds 64. Returns 0 otherwise. The sampled quantity has no proven terrain
 * or height meaning.
 * @source 0x801BB760
 * @status partial
 * @match 65.00
 * @residual The call-result copy now matches; report and scratch gate register allocation differ at the original 80-byte size.
 */
s32 isRouteSampleDifferenceAbove64(s32 arg0, s32 arg1) {
  s32 result;
  s32 value;

  value = func_80154F78(arg0, arg1, g_game_work->route_index_08);
  do {
    result = 0;
  } while (0);
  if (D_1F800000[0] != 0) {
    result = (value << 16) > 0x400000;
  }
  return result;
}
