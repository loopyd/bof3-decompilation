#include "bof3/battle/battle15_internal.h"

/* @source 0x800ABBA4
 * @behavior Runs the battle mode pass dispatchModeHandlerPass (which calls
 *   claimOwnerSlotsRow and then dispatches the mode
 *   handler D_800B5108[D_801462EA]) when the battle mode byte D_801462EA is
 *   nonzero, and the mode-zero pass
 *   setupBattlerActiveSlots when it is clear.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchByBattleMode(void) {
  if (D_801462EA != 0) {
    dispatchModeHandlerPass();
  } else {
    setupBattlerActiveSlots();
  }
}
