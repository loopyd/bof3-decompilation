#include "bof3/ui/game00_internal.h"

/* @kind table */
extern GameEntry0StateHandler D_801CD570[];

/* @behavior dispatches through the handler table at D_801CD570 using the
 * u8 main state machine index at D_80143F4A.
 * @source 0x801C5838
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801C5838(void) {
  D_801CD570[D_80143F4A]();
}
