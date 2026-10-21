#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the third local
 * state-handler table at 0x801C7B7C, indexed by the u16 sub-state word at
 * 0x80143B92.
 * @source 0x80198234
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubstate3(void) {
  D_801C7B7C[D_80143B92]();
}
