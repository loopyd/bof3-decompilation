#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current scenario sub-state through the local
 * state-handler table at 0x801CD568, indexed by the u8 scenario sub-state byte
 * at 0x80143F49.
 * @source 0x801C57F4
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchScenarioSubstate(void) {
  scenarioSubstateHandlerTable[D_80143F49]();
}
