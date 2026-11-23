#include "bof3/battle/battle15_internal.h"

/* @behavior dispatches the current battle selection state through battleSelectionHandlerTable48.
 * @source 0x800A5FF0
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubstateTable4d30(void) {
  battleSelectionHandlerTable48[D_801462E4]();
}
