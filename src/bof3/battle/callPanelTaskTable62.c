#include "bof3/battle/battle15_internal.h"

/* @source 0x800B2088
 * @behavior Dispatches the handler selected by the panel task's state byte.
 * @status exact
 * @match 100.00
 * @residual none
 */

void callPanelTaskTable62(void) {
  battlePanelOuterState[D_80148648->unk_00[2]]();
}
