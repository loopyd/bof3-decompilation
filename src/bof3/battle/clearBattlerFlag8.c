#include "bof3/battle/battle15_internal.h"

/* @source 0x8009DFEC
 * @behavior Applies the 0x8 flags value to the active battler index byte
 *   D_80146394 through func_800A36F0, then runs the selection-input pass
 *   stateRefreshFlagStatus.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearBattlerFlag8(void) {
  func_800A36F0(D_80146394, 8);
  stateRefreshFlagStatus();
}
