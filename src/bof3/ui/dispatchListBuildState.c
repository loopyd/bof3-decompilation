#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers dispatchPriceListState, dispatchPanelState1, dispatchPanelState2 and
 * dispatchPanelState3: reading the cell twice through the shared g_PanelTaskRoot view
 * folds the address into one hoisted callee-saved base. */
#undef D_80148648

/* @source 0x801E4224
 * @behavior panel state dispatcher sibling of dispatchPriceListState, dispatchPanelState1,
 *           dispatchPanelState2 and dispatchPanelState3: calls the handler selected by the
 *           panel task root's state byte (D_801E5F14[D_80148648->state]) with
 *           no arguments (framed jalr), then forwards the same panel task root
 *           to the shop panel step func_801E45C0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchListBuildState(void) {
  D_801E5F14[D_80148648->state]();
  func_801E45C0(D_80148648);
}
