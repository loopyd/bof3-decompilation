#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers dispatchPriceListState, dispatchPanelState1, dispatchPanelState2, dispatchPanelState3,
 * dispatchListBuildState, dispatchPanelFrameState and dispatchFieldIconState: reading the cell twice
 * through the shared panelTaskRoot view folds the address into one hoisted
 * callee-saved base. */
/* @source 0x801E44E0
 * @behavior panel state dispatcher sibling of dispatchPriceListState, dispatchPanelState1,
 *           dispatchPanelState2, dispatchPanelState3, dispatchListBuildState, dispatchPanelFrameState and
 *           dispatchFieldIconState: calls the handler selected by the panel task root
 *           state byte (D_801E5F3C[panelTaskRoot->state]) with no arguments
 *           (framed jalr), then draws the shop ink panel at the same task
 *           root's x and field-6 halfwords through drawShopInkPanel, reading
 *           both fields as signed halfwords so they reach the drawer with `lh`.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchInkPanelState(void) {
  D_801E5F3C[panelTaskRoot->state]();
  drawShopInkPanel((s16)panelTaskRoot->x, (s16)panelTaskRoot->field_06);
}
