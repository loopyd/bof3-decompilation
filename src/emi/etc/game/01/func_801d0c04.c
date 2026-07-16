#include "internal.h"

/* @behavior initializes the frontend overlay state, then runs its perpetual
 * input, state-dispatch, and render/update loop.
 * @source 0x801D0C04
 */
void func_801D0C04(void) {
  GAME_FRONT_STATE = 0u;
  GAME_FRONT_SUBSTATE = 0u;
  func_8014BA04();

  for (;;) {
    func_8014B87C(1);
    func_801D104C();
    GAME_FRONT_STATE_HANDLERS[GAME_FRONT_STATE]();
    func_801D18F8();
    func_801D1B00();
  }
}
