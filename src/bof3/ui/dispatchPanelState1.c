#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatcher dispatchPriceListState: reading the cell twice through the shared
 * g_PanelTaskRoot view folds the address into one hoisted callee-saved base. */
#undef D_80148648

/* @source 0x801E3720
 * @behavior panel state dispatcher sibling of dispatchPriceListState: calls the handler
 *           D_801E5E2C[D_80148648->state] selected by the panel task root's
 *           state byte with no arguments (framed jalr), then forwards the same
 *           panel task root to the shop panel step func_801DC6FC. The three
 *           pointer entries present in the shipped payload are panelNoop
 *           (0x801E2800), statePanelXTo320 and retreatPanelXTo75.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPanelState1(void) {
  D_801E5E2C[D_80148648->state]();
  func_801DC6FC(D_80148648);
}
