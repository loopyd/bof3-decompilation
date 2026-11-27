#include "bof3/battle/battle15_internal.h"

/* @source 0x8009E01C
 * @behavior Applies the 0x100 flags value to the active battler index byte
 *   D_80146394 through stateClearStatusBits, then runs the selection-input pass
 *   stateRefreshFlagStatus.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearBattlerFlag100(void) {
  stateClearStatusBits(D_80146394, 0x100);
  stateRefreshFlagStatus();
}
