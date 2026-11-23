#include "bof3/battle/battle15_internal.h"

/* @source 0x800A59AC
 * @behavior dispatches the current battle-selection handler from battleSelectionHandlerTable47.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubstateTable4d14(void) {
  battleSelectionHandlerTable47[D_801462E4]();
}
