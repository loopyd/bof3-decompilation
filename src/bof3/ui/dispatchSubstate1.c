#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the first local
 * state-handler table.
 * @source 0x801975E4
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void dispatchSubstate1(void) {
  u16 state;

  state = D_80143B92;
  D_801C7B44[state]();
}
