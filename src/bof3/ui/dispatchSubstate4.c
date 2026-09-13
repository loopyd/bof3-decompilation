#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the fourth local
 * state-handler table.
 * @source 0x801984AC
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void dispatchSubstate4(void) {
  u16 state;

  state = D_80143B92;
  D_801C7B88[state]();
}
