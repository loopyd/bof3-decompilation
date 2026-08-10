#include "bof3/ui/game00_internal.h"

/* @behavior calls the scenario handler selected by scenarioState.scenario_id
 * through the handler-pointer table at D_801C8454 (entries point at records
 * in a companion overlay whose first field is the handler), then dispatches
 * the main state machine.
 * @source 0x801A782C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchScenarioHandlerAndState(void) {
  (*D_801C8454[scenarioState.scenario_id])();
  dispatchStateHandler();
}
