#include "bof3/world/area02613_internal.h"

/**
 * @source 0x801F305C
 * @behavior Resets the area state and selects scenario progress from the area flags.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801F305C(void) {
  s32 flags;

  flags = D_801490C7;
  D_801490A8 = 0xFFFF;
  switch (flags) {
  case 0:
    g_ScenarioProgress = 8;
    break;
  case 1:
    g_ScenarioProgress = 15;
    break;
  }
}
