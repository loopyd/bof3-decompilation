#include "bof3/ui/game00_internal.h"

/* @source 0x801C1DF0
 * @behavior dispatches the indexed handler for the active scenario
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801C1DF0(u8 index) {
  D_801CD4C0[scenarioState.scenario_id][index]();
}
