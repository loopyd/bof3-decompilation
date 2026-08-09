#include "bof3/battle/battle15_internal.h"

/* @source 0x800B2088
 * @behavior Dispatches the handler selected by the panel task's state byte.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */

void dispatchPanelTaskTable6e08(void) {
  D_800B6E08[D_80148648->unk_00[2]]();
}
