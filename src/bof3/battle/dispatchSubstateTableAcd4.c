#include "bof3/battle/battle03_internal.h"

/* @source 0x801D6774
 * @behavior dispatches through the shared byte-selected battle handler table.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTableAcd4(void) {
  D_801EACD4[D_801462E1[0]]();
}
