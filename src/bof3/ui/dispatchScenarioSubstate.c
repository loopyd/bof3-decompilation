#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current scenario sub-state through the local
 * state-handler table at 0x801CD568.
 * @source 0x801C57F4
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void dispatchScenarioSubstate(void) {
  GameEntry0StateHandler callback;
  u8                     state;

  state = D_80143F49;
  callback = scenarioSubstateHandlerTable[state];
  callback();
}
