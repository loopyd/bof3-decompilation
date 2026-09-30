#include "bof3/ui/game00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers func_801E29C8/func_801E2AB0: reading the cell twice through
 * the shared g_PanelTaskRoot view folds the address into one hoisted
 * callee-saved base. */
#undef D_80148648

/* @source 0x801995A4
 * @behavior front-end panel state dispatcher: calls the handler selected by the
 * panel task root state byte (D_801C7C00[D_80148648->state]) with no arguments
 * (framed jalr), then runs the panel task update over the same task root
 * through func_801999F8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801995A4(void) {
  D_801C7C00[D_80148648->state]();
  func_801999F8(D_80148648);
}
