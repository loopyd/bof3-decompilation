#include "bof3/battle/battle15_internal.h"

/* @source 0x800ABBE0
 * @behavior Runs the battle substate pass
 *   claimOwnerSlotsInModeRowThenSeedCountdown, then dispatches the handler
 *   selected by the battle mode byte D_801462EA through the function table
 *   rooted at D_800B5108.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800ABBE0(void) {
  claimOwnerSlotsInModeRowThenSeedCountdown();
  D_800B5108[D_801462EA]();
}
