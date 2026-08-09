#include "bof3/battle/battle15_internal.h"

/* @source 0x80098B08
 * @behavior dispatches a byte-selected battle handler.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4458(void) {
  D_800B4458[D_801462E4]();
}
