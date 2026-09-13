#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the third local
 * state-handler table.
 * @source 0x80198234
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void dispatchSubstate3(void) {
  GameEntry0StateHandler callback;
  u16                    state;

  state = D_80143B92;
  callback = D_801C7B7C[state];
  callback();
}
