#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers dispatchPriceListState, dispatchPanelState1, dispatchPanelState2 and
 * dispatchPanelState3: reading the cell twice through the shared panelTaskRoot view
 * folds the address into one hoisted callee-saved base. */
/* @source 0x801E3E4C
 * @behavior panel state dispatcher sibling of dispatchPriceListState, dispatchPanelState1,
 *           dispatchPanelState2, dispatchPanelState3 and func_801E4224: calls the handler
 *           D_801E5E70[panelTaskRoot->state] selected by the panel task root state
 *           byte with no arguments (framed jalr), then forwards the same panel
 *           task root to the shop field-list emitter func_801D826C with its x
 *           and field-6 halfwords (lhu), its byte 0xA (lbu) and the constants
 *           0 and 0, the last one on the stack.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchFieldListState(void) {
  D_801E5E70[panelTaskRoot->state]();
  func_801D826C(panelTaskRoot->x, panelTaskRoot->field_06,
                *((u8*)((u8*)panelTaskRoot) + 0xA), 0, 0);
}
