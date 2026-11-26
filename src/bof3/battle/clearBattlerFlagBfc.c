#include "bof3/battle/battle15_internal.h"

/* @source 0x8009E04C
 * @behavior Applies the 0xBFC flags value to the active battler index byte
 *   D_80146394 through func_800A36F0, then runs the selection-input pass
 *   stateRefreshFlagStatus.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearBattlerFlagBfc(void) {
  func_800A36F0(D_80146394, 0xBFC);
  stateRefreshFlagStatus();
}
