#include "bof3/ui/shop00_internal.h"

/* Both accesses need the target-local symbol relocation, matching the sibling
 * state dispatchers func_801E2DB4 and func_801E2AB0: reading the cell twice
 * through the shared g_PanelTaskRoot view folds the address into one hoisted
 * callee-saved base. */
#undef D_80148648

/* @source 0x801E2C18
 * @behavior panel state dispatcher sibling of func_801E2DB4: calls the handler
 *           selected by the panel task root state byte
 *           (D_801E5DDC[D_80148648->state]) with no arguments (framed jalr),
 *           then forwards the same task root's x and field-6 halfwords, the
 *           field-record index D_80181B10[D_80144F5A[task byte 0xA]], task byte
 *           0xB and task byte 0xC to the shop field-record emitter
 *           func_801DB9F0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E2C18(void) {
  D_801E5DDC[D_80148648->state]();
  func_801DB9F0(D_80148648->x, D_80148648->field_06,
                D_80181B10[D_80144F5A[*((u8*)D_80148648 + 0xA)]],
                *((u8*)D_80148648 + 0xB), *((u8*)D_80148648 + 0xC));
}
