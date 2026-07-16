#include "internal.h"

/* @behavior requests the scenario overlay selected by the signed scenario ID.
 * @source 0x801A7804
 */
void func_801A7804(void) {
  func_80161FDC(GAME_SCENARIO_STATE.scenario_id + 661);
}
