#include "bof3/ui/game00_internal.h"

/*
 * @behavior Requires selection record byte 0x79 to be outside {2, 4, 9, 10},
 *           the two work gate words at 0x80145AA8 / 0x80145AC0 to share a set
 *           bit and front flag bit 0 at 0x80143F02 to be clear; then sets work
 *           area byte 0x01 to 10, clears work area byte 0x02 and sets
 *           selection record byte 0x12B to 8, returning 1. Every failed gate
 *           returns 0.
 * @source 0x801B5EBC
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801B5EBC(void) {
  u8 panel_mode = D_80146250[0x79];

  if (panel_mode == 2 || panel_mode == 4) {
    return 0;
  }
  if (panel_mode == 9 || panel_mode == 10) {
    return 0;
  }
  if ((D_80145AA8 & D_80145AC0) == 0) {
    return 0;
  }
  if (D_80143F02 & 1) {
    return 0;
  }
  g_game_work->unk_01 = 10;
  g_game_work->flags_02 = 0;
  D_80146250[0x12B] = 8;
  return 1;
}
