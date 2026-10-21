#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the first local
 * state-handler table at 0x801C7B44, indexed by the u16 sub-state word at
 * 0x80143B92.
 * @source 0x801975E4
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubstate1(void) {
  D_801C7B44[D_80143B92]();
}
