#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the seventh local
 * state-handler table at 0x801C7BB0, indexed by the u16 sub-state word at
 * 0x80143B92.
 * @source 0x80198AC4
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubstate7(void) {
  D_801C7BB0[D_80143B92]();
}
