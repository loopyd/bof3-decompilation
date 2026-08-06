#include "internal.h"

/* @behavior requests the scenario overlay selected by the signed scenario ID.
 * @source 0x801A7804
 */
void requestScenarioOverlay(void) {
  initStreamSlot(scenarioState.scenario_id + 661);
}
