#include "internal.h"

/* @behavior requests the scenario overlay selected by the signed scenario ID.
 * @source 0x801a7804 FUN_801a7804
 */
void func_801a7804(void) {
  func_80161fdc(GAME_SCENARIO_STATE.scenario_id + 661);
}
