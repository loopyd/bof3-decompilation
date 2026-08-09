#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the sixth local
 * state-handler table.
 * @source 0x80198904
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstate6(void) {
  u16 state;

  state = D_80143B92;
  barrier();
  D_801C7BA4[state]();
}
