#include "bof3/battle/battle15_internal.h"

/* @source 0x800A4900
 * @behavior dispatches the byte-selected battle handler.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4cd0(void) {
  battleSelectionHandlerTable4CD0[D_801462E4]();
}
