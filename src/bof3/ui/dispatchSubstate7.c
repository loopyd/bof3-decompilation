#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the seventh local
 * state-handler table.
 * @source 0x80198AC4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstate7(void) {
  u16 state;

  state = D_80143B92;
  barrier();
  D_801C7BB0[state]();
}
