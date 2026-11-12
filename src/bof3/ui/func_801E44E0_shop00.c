#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers func_801E2EB0, func_801E3720, func_801E3BA4, func_801E3CF8,
 * func_801E4224, func_801E4338 and func_801E440C: reading the cell twice
 * through the shared g_PanelTaskRoot view folds the address into one hoisted
 * callee-saved base. */
#undef D_80148648

/* @source 0x801E44E0
 * @behavior panel state dispatcher sibling of func_801E2EB0, func_801E3720,
 *           func_801E3BA4, func_801E3CF8, func_801E4224, func_801E4338 and
 *           func_801E440C: calls the handler selected by the panel task root
 *           state byte (D_801E5F3C[D_80148648->state]) with no arguments
 *           (framed jalr), then draws the shop ink panel at the same task
 *           root's x and field-6 halfwords through drawShopInkPanel, reading
 *           both fields as signed halfwords so they reach the drawer with `lh`.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E44E0(void) {
  D_801E5F3C[D_80148648->state]();
  drawShopInkPanel((s16)D_80148648->x, (s16)D_80148648->field_06);
}
