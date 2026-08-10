#include "bof3/ui/game00_internal.h"

/* @behavior stores 0xF0 to g_ScenarioProgress and returns zero.
 * @source 0x801A8714
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 setScenarioProgressToF0(void) {
  g_ScenarioProgress = 0xF0;
  return 0;
}
