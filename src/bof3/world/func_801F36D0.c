#include "bof3/world/area02713_internal.h"

/**
 * @source 0x801F36D0
 * @behavior Resets the area state and selects scenario progress from the area flags.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801F36D0(void) {
  s32 flags;

  flags = D_801490C7;
  WORLD00_AREA027_STATE_90A8 = 0xFFFF;
  switch (flags) {
  case 0:
    g_ScenarioProgress = 92;
    break;
  case 1:
    g_ScenarioProgress = 93;
    break;
  }
}
