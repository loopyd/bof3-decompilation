#include "bof3/battle/battle15_internal.h"

/* @source 0x800A4700
 * @behavior Selection-handler step 0: while the battle selection state byte
 *   D_80148573 is still 0, starts EMI stream slot 0 through func_80161FDC and
 *   advances the selection handler index D_801462E4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initSelectionStreamSlotWhenStateClear(void) {
  u8* counter;
  u8  value;

  if (D_80148573 != 0) {
    return;
  }

  func_80161FDC(0);
  counter = &D_801462E4;
  value = *counter;
  *counter = value + 1;
}
