#include "bof3/world/area02713_internal.h"

/*
 * @source 0x801F3690
 * @behavior Clears area state and maps area flag 0 or 1 to scenario progress 91 or 90.
 */
void func_801F3690(void) {
  D_801490A8 = 0xFFFF;
  switch (D_801490C7) {
  case 0:
    g_ScenarioProgress = 91;
    break;
  case 1:
    g_ScenarioProgress = 90;
    break;
  default:
    break;
  }
}
