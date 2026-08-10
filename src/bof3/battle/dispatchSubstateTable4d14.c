#include "bof3/battle/battle15_internal.h"

/* @source 0x800A59AC
 * @behavior dispatches the current battle-selection handler from battleSelectionHandlerTable4D14.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4d14(void) {
  battleSelectionHandlerTable4D14[D_801462E4]();
}
