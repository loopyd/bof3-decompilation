#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, rather than the shared
 * constant view used by the other panel task code: reading the cell twice
 * through g_PanelTaskRoot folds the address into one hoisted callee-saved base. */
#undef D_80148648

/* @source 0x801E2EB0
 * @behavior panel state dispatcher: calls the handler selected by the panel
 *           task root's state byte (D_801E5DFC[D_80148648->state]) with no
 *           arguments (framed jalr), then steps the same panel task root
 *           through func_801E2F04.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E2EB0(void) {
  D_801E5DFC[D_80148648->state]();
  func_801E2F04(D_80148648);
}
