#include "bof3/ui/game00_internal.h"

/* @behavior requests the scenario overlay selected by the signed scenario ID.
 * @source 0x801A7804
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void requestScenarioOverlay(void) {
  initStreamSlot(scenarioState.scenario_id + 661);
}
