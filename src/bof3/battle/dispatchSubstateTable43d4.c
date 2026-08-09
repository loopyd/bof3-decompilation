#include "bof3/battle/battle15_internal.h"

/* @source 0x80096F00
 * @behavior dispatches the byte-selected battle handler.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable43d4(void) {
  D_800B43D4[D_801462E3]();
}
