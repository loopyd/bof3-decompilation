#include "internal.h"

/* @behavior requests the scenario overlay selected by the signed scenario ID.
 * @source 0x801A7804
 */
void game_scenario_request_overlay(void) {
  emi_stream_init_slot(GAME_SCENARIO_STATE.scenario_id + 661);
}
