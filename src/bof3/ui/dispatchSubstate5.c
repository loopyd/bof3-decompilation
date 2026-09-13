#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the fifth local
 * state-handler table.
 * @source 0x80198744
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void dispatchSubstate5(void) {
  u16 state;

  state = D_80143B92;
  D_801C7B98[state]();
}
