#include "bof3/ui/game00_internal.h"

/* Both accesses need the target-local symbol relocation rather than the shared
 * panelTaskRoot constant view: reading the panel task root cell twice through
 * the shared view folds its address into one hoisted callee-saved base. */
/* @source 0x801996A8
 * @behavior front-end panel state dispatcher: calls the handler selected by the
 * panel task root state byte (D_801C7C0C[panelTaskRoot->state]) with no arguments
 * (framed jalr), then runs the panel task update over the same task root
 * through func_801D7AD0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callPanelStateHandler2(void) {
  D_801C7C0C[panelTaskRoot->state]();
  func_801D7AD0(panelTaskRoot);
}
