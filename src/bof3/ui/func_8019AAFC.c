#include "bof3/ui/game00_internal.h"

/**
 * @source 0x8019AAFC
 * @behavior Saves work coordinates and clears scenario/work flags.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8019AAFC(void) {
  s32* coord_68;
  s32* coord_6C;
  struct GameWorkArea* work;
  u8* scenario_flag;

  work = g_game_work;
  D_801492D8 = work->coord_64;
  coord_68 = &work->coord_68;
  D_801492DA = *coord_68;
  coord_6C = &work->coord_6C;
  scenario_flag = &scenarioState.field_01;
  coord_6C = &(*coord_6C);
  D_801492DC = *coord_6C;
  *scenario_flag &= 0xDF;
  clearWorkFlags();
}
