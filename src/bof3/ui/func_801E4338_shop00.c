#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers func_801E2EB0, func_801E3720, func_801E3BA4, func_801E3CF8
 * and func_801E4224: reading the cell twice through the shared g_PanelTaskRoot
 * view folds the address into one hoisted callee-saved base. */
#undef D_80148648

/* @source 0x801E4338
 * @behavior panel state dispatcher sibling of func_801E2EB0, func_801E3720,
 *           func_801E3BA4, func_801E3CF8 and func_801E4224: calls the handler
 *           selected by the panel task root state byte
 *           (D_801E5F24[D_80148648->state]) with no arguments (framed jalr),
 *           then forwards the same panel task root to the shop panel emitter
 *           func_801E4DF4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E4338(void) {
  D_801E5F24[D_80148648->state]();
  func_801E4DF4(D_80148648);
}
