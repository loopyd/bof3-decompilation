#include "internal.h"

/**
 * @source 0x801C1E3C
 * @behavior Dispatches the indexed handler for the current scenario.
 */
void func_801C1E3C(u8 index)
{
  D_801CD510[scenarioState.scenario_id][index]();
}
