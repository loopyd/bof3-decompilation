#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x8009DA94
 * @behavior Invoke the local battle update helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callBattleFlagUpdate(void)
{
  stateRefreshFlagStatus();
}
