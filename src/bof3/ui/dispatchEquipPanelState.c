#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers dispatchCommandRowState, dispatchMoneyPanelState and func_801E2EB0: reading
 * the cell twice through the shared panelTaskRoot view folds the address
 * into one hoisted callee-saved base. */
/* @source 0x801E2DB4
 * @behavior panel state dispatcher sibling of dispatchCommandRowState and
 *           dispatchMoneyPanelState: calls the handler selected by the panel task root
 *           state byte (D_801E5DF0[panelTaskRoot->state]) with no arguments
 *           (framed jalr), then draws the shop panel through
 *           drawShopEquipPanel with the same task root's x and field-6
 *           halfwords and the main-RAM byte D_80181B10[D_80144F5A[task byte
 *           0xA]].
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchEquipPanelState(void) {
  D_801E5DF0[panelTaskRoot->state]();
  drawShopEquipPanel(panelTaskRoot->x, panelTaskRoot->field_06,
                     D_80181B10[D_80144F5A[*((u8*)((u8*)panelTaskRoot) + 0xA)]]);
}
