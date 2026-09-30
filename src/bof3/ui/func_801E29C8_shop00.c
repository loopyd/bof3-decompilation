#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers func_801E2AB0, func_801E2EB0 and func_801E44E0: reading
 * the cell twice through the shared g_PanelTaskRoot view folds the address
 * into one hoisted callee-saved base. */
#undef D_80148648

/* @source 0x801E29C8
 * @behavior panel state dispatcher sibling of func_801E2AB0, func_801E2EB0,
 *           func_801E3720 and func_801E44E0: calls the handler selected by the
 *           panel task root state byte (D_801E5DC0[D_80148648->state]) with no
 *           arguments (framed jalr), then draws the shop panel through
 *           func_801DA77C with the same task root's signed x and field-6
 *           halfwords, its bytes 0xA and 0xB and the constant 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E29C8(void) {
  D_801E5DC0[D_80148648->state]();
  func_801DA77C(D_80148648->x, D_80148648->field_06,
                *((u8*)D_80148648 + 0xA), *((u8*)D_80148648 + 0xB), 0);
}
