#include "bof3/ui/game00_internal.h"

/* @behavior Selects the scratch work-area route index from the top nibble of
 * the shared frontend word D_8014625C: 0x1000 stores 0, 0x2000 stores 2,
 * 0x3000 stores 1, 0x4000 stores 4, 0x6000 stores 3, 0x8000 stores 6, 0x9000
 * stores 7 and 0xC000 stores 5 into route_index_08, and every handled nibble
 * returns 1; any other nibble returns 0 without touching the work area.
 * @source 0x801B59B4
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801B59B4(void) {
  switch (D_8014625C & 0xF000) {
  case 0x1000:
    g_game_work->route_index_08 = 0;
    break;
  case 0x3000:
    g_game_work->route_index_08 = 1;
    break;
  case 0x2000:
    g_game_work->route_index_08 = 2;
    break;
  case 0x6000:
    g_game_work->route_index_08 = 3;
    break;
  case 0x4000:
    g_game_work->route_index_08 = 4;
    break;
  case 0xC000:
    g_game_work->route_index_08 = 5;
    break;
  case 0x8000:
    g_game_work->route_index_08 = 6;
    break;
  case 0x9000:
    g_game_work->route_index_08 = 7;
    break;
  default:
    return 0;
  }
  return 1;
}
