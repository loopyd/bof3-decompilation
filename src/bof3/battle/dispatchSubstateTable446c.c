#include "bof3/battle/battle15_internal.h"

/* @source 0x80099178
 * @behavior dispatches a byte-selected battle handler.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable446c(void) {
  D_800B446C[D_801462E4]();
}
