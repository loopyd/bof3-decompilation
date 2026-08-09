#include "bof3/battle/battle15_internal.h"

/* @source 0x8009834C
 * @behavior dispatches the byte-selected battle handler.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4428(void) {
  D_800B4428[D_801462E3]();
}
