#include "bof3/world/area02713_internal.h"

/**
 * @source 0x801F3650
 * @behavior Resets the area state; flags 0 and 1 set scenario progress to 5 and 6 respectively.
 * @status exact
 * @match 100.00
 */
void func_801F3650(void) {
  s32 flags;

  flags = D_801490C7;
  WORLD00_AREA027_STATE_90A8 = 0xFFFF;
  switch (flags) {
  case 0:
    g_ScenarioProgress = 5;
    break;
  case 1:
    g_ScenarioProgress = 6;
    break;
  default:
    break;
  }
}
