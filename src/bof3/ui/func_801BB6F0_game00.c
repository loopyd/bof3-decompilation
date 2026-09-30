#include "bof3/ui/game00_internal.h"

/* @source 0x801BB6F0
 * @behavior Reports whether arg0 selects the current route index: the high
 *           nibble must be 0xB0 and the low nibble must equal the scratch work
 *           area route_index_08, except that nibble 8 instead accepts route
 *           index 3 or 5.
 * @status partial
 * @match 75.86
 * @residual First differing byte is the fail-branch displacement at +0x0008, caused by two
 *           extra dense-arm instructions: the low-nibble equality is materialized as its own
 *           beq to the shared return-1 block plus a reload of the constant 8, where the
 *           original jumps straight into the block it shares with the sparse chain's
 *           route != 5 test. 112->116 bytes; clean-C equality block-merge residual.
 */
s32 func_801BB6F0(u32 arg0) {
  if ((arg0 & 0xF0) != 0xB0) {
    return 0;
  }
  if ((arg0 & 0x0F) < 6 && g_game_work->route_index_08 == (arg0 & 0x0F)) {
    return 1;
  }
  if ((arg0 & 0x0F) == 8 &&
      (g_game_work->route_index_08 == 3 ||
       g_game_work->route_index_08 == 5)) {
    return 1;
  }
  return 0;
}
