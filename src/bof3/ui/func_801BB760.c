#include "bof3/ui/game00_internal.h"

/* @behavior Queries the shared route-parameterised position service
 *           func_80154F78 with the two incoming coordinates and the scratch
 *           work-area route index, then reports whether the returned value
 *           shifted up by 16 bits exceeds 0x400000; the report is suppressed to
 *           zero while the scratchpad byte at 0x1F800000 is zero.
 * @source 0x801BB760
 * @status partial
 * @match 65.00
 * @residual The call-result copy now matches; report and scratch gate register allocation differ at the original 80-byte size.
 */
s32 func_801BB760(s32 arg0, s32 arg1) {
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
