#include "bof3/battle/battle15_internal.h"

/* @source 0x800A4C08
 * @behavior dispatches the byte-selected battle handler through the local table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSubstateTable4ce4(void) {
  battleSelectionHandlerTable4CE4[D_801462E4]();
}
