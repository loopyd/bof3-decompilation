#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, rather than the shared
 * constant view used by the other panel task code: reading the cell twice
 * through panelTaskRoot folds the address into one hoisted callee-saved base. */
/* @source 0x801E3BA4
 * @behavior panel state dispatcher sibling of dispatchPriceListState and dispatchPanelState1:
 *           calls the handler selected by the panel task root's state byte
 *           (D_801E5E48[panelTaskRoot->state]) with no arguments (framed jalr),
 *           then forwards the same panel task root to the shop panel step
 *           func_801E112C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPanelState2(void) {
  D_801E5E48[panelTaskRoot->state]();
  func_801E112C(panelTaskRoot);
}
