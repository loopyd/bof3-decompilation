#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatcher dispatchPriceListState: reading the cell twice through the shared
 * panelTaskRoot view folds the address into one hoisted callee-saved base. */
/* @source 0x801E2AB0
 * @behavior panel state dispatcher sibling of dispatchPriceListState: calls the handler
 *           D_801E5DCC[panelTaskRoot->state] selected by the panel task root state
 *           byte with no arguments (framed jalr), then draws the shop money
 *           panel through drawShopMoneyPanel with the same task root's x and
 *           field-6 halfwords, the constant 0 and the main-RAM value
 *           D_80144F50.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchMoneyPanelState(void) {
  D_801E5DCC[panelTaskRoot->state]();
  drawShopMoneyPanel(panelTaskRoot->x, panelTaskRoot->field_06, 0, D_80144F50);
}
