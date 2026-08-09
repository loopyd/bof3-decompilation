#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the first local
 * state-handler table.
 * @source 0x801975E4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstate1(void) {
  u16 state;

  state = D_80143B92;
  barrier();
  D_801C7B44[state]();
}
