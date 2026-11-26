#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers dispatchMoneyPanelState, dispatchPriceListState and dispatchInkPanelState: reading
 * the cell twice through the shared panelTaskRoot view folds the address
 * into one hoisted callee-saved base. */
/* @source 0x801E29C8
 * @behavior panel state dispatcher sibling of dispatchMoneyPanelState, dispatchPriceListState,
 *           dispatchPanelState1 and dispatchInkPanelState: calls the handler selected by the
 *           panel task root state byte (D_801E5DC0[panelTaskRoot->state]) with no
 *           arguments (framed jalr), then draws the shop command row through
 *           drawShopCommandRow with the same task root's x and field-6
 *           halfwords, its bytes 0xA and 0xB and the constant 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchCommandRowState(void) {
  D_801E5DC0[panelTaskRoot->state]();
  drawShopCommandRow(panelTaskRoot->x, panelTaskRoot->field_06,
                    *((u8*)((u8*)panelTaskRoot) + 0xA), *((u8*)((u8*)panelTaskRoot) + 0xB), 0);
}
