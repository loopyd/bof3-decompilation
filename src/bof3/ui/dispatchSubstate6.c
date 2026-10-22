#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the sixth local
 * state-handler table at 0x801C7BA4, indexed by the u16 sub-state word at
 * 0x80143B92.
 * @source 0x80198904
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubstate6(void) {
  D_801C7BA4[D_80143B92]();
}
