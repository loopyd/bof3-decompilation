#include "internal.h"

/* @behavior initializes the frontend overlay state, then runs its perpetual
 * input, state-dispatch, and render/update loop.
 * @source 0x801d0c04 FUN_801d0c04
 */
void func_801d0c04(void) {
  GAME_FRONT_STATE = 0u;
  GAME_FRONT_SUBSTATE = 0u;
  func_8014ba04();

  for (;;) {
    func_8014b87c(1);
    func_801d104c();
    GAME_FRONT_STATE_HANDLERS[GAME_FRONT_STATE]();
    func_801d18f8();
    func_801d1b00();
  }
}
