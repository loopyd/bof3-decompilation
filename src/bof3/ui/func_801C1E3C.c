#include "bof3/ui/game00_internal.h"

/**
 * @source 0x801C1E3C
 * @behavior Dispatches the indexed handler for the current scenario.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801C1E3C(u8 index)
{
  D_801CD510[scenarioState.scenario_id][index]();
}
