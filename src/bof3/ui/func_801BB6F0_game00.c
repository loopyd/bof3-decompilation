#include "bof3/ui/game00_internal.h"

/* @source 0x801BB6F0
 * @behavior Reports whether arg0 selects the current route index: the high
 *           nibble must be 0xB0 and the low nibble must equal the scratch work
 *           area route_index_08, except that nibble 8 instead accepts route
 *           index 3 or 5.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801BB6F0(u32 arg0) {
  u32 nibble;

  if ((arg0 & 0xF0) != 0xB0) {
    return 0;
  }
  nibble = arg0 & 0x0F;
  if (nibble < 6) {
    if (g_game_work->route_index_08 != nibble) {
      return 0;
    }
  } else {
    if (nibble != 8) {
      return 0;
    }
    if (g_game_work->route_index_08 != 3 &&
        g_game_work->route_index_08 != 5) {
      return 0;
    }
  }
  return 1;
}
