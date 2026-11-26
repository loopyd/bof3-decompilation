#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers dispatchPriceListState, dispatchPanelState1, dispatchPanelState2, dispatchPanelState3
 * and dispatchListBuildState: reading the cell twice through the shared panelTaskRoot
 * view folds the address into one hoisted callee-saved base. */
/* @source 0x801E4338
 * @behavior panel state dispatcher sibling of dispatchPriceListState, dispatchPanelState1,
 *           dispatchPanelState2, dispatchPanelState3 and dispatchListBuildState: calls the handler
 *           selected by the panel task root state byte
 *           (D_801E5F24[panelTaskRoot->state]) with no arguments (framed jalr),
 *           then forwards the same panel task root to the shop panel emitter
 *           func_801E4DF4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPanelFrameState(void) {
  D_801E5F24[panelTaskRoot->state]();
  func_801E4DF4(panelTaskRoot);
}
