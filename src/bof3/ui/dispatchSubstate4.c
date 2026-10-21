#include "bof3/ui/game00_internal.h"

/* @behavior dispatches the current front-end sub-state through the fourth local
 * state-handler table at 0x801C7B88, indexed by the u16 sub-state word at
 * 0x80143B92.
 * @source 0x801984AC
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubstate4(void) {
  D_801C7B88[D_80143B92]();
}
