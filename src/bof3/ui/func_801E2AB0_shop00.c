#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatcher func_801E2EB0: reading the cell twice through the shared
 * g_PanelTaskRoot view folds the address into one hoisted callee-saved base. */
#undef D_80148648

/* @source 0x801E2AB0
 * @behavior panel state dispatcher sibling of func_801E2EB0: calls the handler
 *           D_801E5DCC[D_80148648->state] selected by the panel task root state
 *           byte with no arguments (framed jalr), then draws the shop panel
 *           through func_801DA2F4 with the same task root's x and field-6
 *           halfwords, the constant 0 and the main-RAM value D_80144F50.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E2AB0(void) {
  D_801E5DCC[D_80148648->state]();
  func_801DA2F4(D_80148648->x, D_80148648->field_06, 0, D_80144F50);
}
