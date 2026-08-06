#include "internal.h"

/* @behavior dispatches the current scenario sub-state through the local
 * state-handler table at 0x801CD568.
 * @source 0x801C57F4
 */
void game_scenario_dispatch_substate(void) {
  GameEntry0StateHandler callback;
  u8                     state;

  state = D_80143F49;
  callback = game_scenario_substate_handlerTable[state];
  barrier();
  callback();
}
