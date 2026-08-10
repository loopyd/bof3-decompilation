#include "bof3/world/area02613_internal.h"

/**
 * @source 0x801F300C
 * @behavior Selects a table value and scenario progress from the signed area mode.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801F300C(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F4CDC[mode];
  switch (mode) {
  case 0:
    g_ScenarioProgress = 0xBE;
    break;
  case 1:
    g_ScenarioProgress = 0xBF;
    break;
  }
}
