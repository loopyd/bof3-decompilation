#include "bof3/battle/battle15_internal.h"

/* @source 0x800980A8
 * @behavior dispatches the byte-selected battle handler.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4418(void) {
  D_800B4418[D_801462E3]();
}
