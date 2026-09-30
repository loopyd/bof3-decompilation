#include "bof3/ui/game00_internal.h"

/* Both accesses need the target-local symbol relocation rather than the shared
 * g_PanelTaskRoot constant view: reading the panel task root cell twice through
 * the shared view folds its address into one hoisted callee-saved base. */
#undef D_80148648

/* @source 0x801996A8
 * @behavior front-end panel state dispatcher: calls the handler selected by the
 * panel task root state byte (D_801C7C0C[D_80148648->state]) with no arguments
 * (framed jalr), then runs the panel task update over the same task root
 * through func_801D7AD0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801996A8(void) {
  D_801C7C0C[D_80148648->state]();
  func_801D7AD0(D_80148648);
}
