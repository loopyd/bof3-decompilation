#include "bof3/battle/battle15_internal.h"

/* @source 0x800A52C4
 * @behavior dispatches the byte-selected battle handler from battleSelectionHandlerTable4D00.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4d00(void) {
  battleSelectionHandlerTable4D00[D_801462E4]();
}
