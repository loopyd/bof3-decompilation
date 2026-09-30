#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, rather than the shared
 * constant view used by the other panel task code: reading the cell twice
 * through g_PanelTaskRoot folds the address into one hoisted callee-saved base. */
#undef D_80148648

/* @source 0x801E3CF8
 * @behavior panel state dispatcher sibling of func_801E2EB0, func_801E3720 and
 *           func_801E3BA4: calls the handler D_801E5E5C[D_80148648->state]
 *           selected by the panel task root state byte with no arguments
 *           (framed jalr), then forwards the same panel task root to the shop
 *           panel step func_801E1710.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E3CF8(void) {
  D_801E5E5C[D_80148648->state]();
  func_801E1710(D_80148648);
}
