#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, rather than the shared
 * constant view used by the other panel task code: reading the cell twice
 * through panelTaskRoot folds the address into one hoisted callee-saved base. */
/* @source 0x801E2EB0
 * @behavior panel state dispatcher: calls the handler selected by the panel
 *           task root's state byte (D_801E5DFC[panelTaskRoot->state]) with no
 *           arguments (framed jalr), then steps the same panel task root
 *           through drawShopPriceList.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPriceListState(void) {
  D_801E5DFC[panelTaskRoot->state]();
  drawShopPriceList(panelTaskRoot);
}
