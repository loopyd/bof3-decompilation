#include "bof3/battle/battle15_internal.h"

/* @source 0x8009DD20
 * @behavior UNKNOWN: exact behavior is not yet documented.
 * @status exact
 * @match 100.00
 * @residual none
 */

void clearBattlerFlag80(void) {
  stateClearStatusBits(D_80146394, 0x80);
  stateRefreshFlagStatus();
}
