#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers func_801E2EB0, func_801E3720, func_801E3BA4 and
 * func_801E3CF8: reading the cell twice through the shared g_PanelTaskRoot view
 * folds the address into one hoisted callee-saved base. */
#undef D_80148648

/* @source 0x801E3E4C
 * @behavior panel state dispatcher sibling of func_801E2EB0, func_801E3720,
 *           func_801E3BA4, func_801E3CF8 and func_801E4224: calls the handler
 *           D_801E5E70[D_80148648->state] selected by the panel task root state
 *           byte with no arguments (framed jalr), then forwards the same panel
 *           task root to the shop field-list emitter func_801D826C with its x
 *           and field-6 halfwords (lhu), its byte 0xA (lbu) and the constants
 *           0 and 0, the last one on the stack.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E3E4C(void) {
  D_801E5E70[D_80148648->state]();
  func_801D826C(D_80148648->x, D_80148648->field_06,
                *((u8*)D_80148648 + 0xA), 0, 0);
}
