#include "internal.h"

/* @behavior advances the front-end into state 2 when the shared input/status
 * flag is set, then runs the common state update.
 * @source 0x801979D4
 */
void game_front_enter_state_2_on_input(void) {
  if ((D_801490A4 & 2u) != 0u) {
    D_80146256 = 0x10u;
    D_80143BB0 = 0u;
    D_80143B90 = 2u;
  }

  func_80199230();
}
