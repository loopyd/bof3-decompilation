#include "internal.h"

/* @behavior initializes the frontend overlay state, then runs its perpetual
 * input, state-dispatch, and render/update loop.
 * @source 0x801D0C04
 */
void game_front_main_loop(void) {
  GAME_FRONT_STATE = 0u;
  GAME_FRONT_SUBSTATE = 0u;
  func_8014BA04();

  for (;;) {
    func_8014B87C(1);
    game_front_pre_dispatch_gate();
    GAME_FRONT_STATE_HANDLERS[GAME_FRONT_STATE]();
    game_front_update_banner();
    game_front_update_windows();
  }
}
