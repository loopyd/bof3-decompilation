#include "bof3/battle/battle03_internal.h"

/* @source 0x801E9F18
 * @behavior Second of the three panel-task root handlers dispatched by panel
 * byte +0x03 through the table copied off 0x801D0FE0 (func_801E9B18). It
 * returns while the battle selection gate byte 0x801462EF is clear. With the
 * gate set it takes panel byte +0x0A as a battler index and returns only when
 * isBattlerInactiveOrFlag4000 reports that battler inactive while both
 * selection setup predicates report disabled; otherwise it activates the panel
 * task for the queued selection slot whose first byte D_801EB4D8 equals panel
 * byte +0x0A and again when that slot byte carries bit 0x80.
 * @status exact
 * @match 100.00
 * @residual none
 */
void activatePanelTaskUnlessBattlerInactive(void) {
  if (D_801462EF == 0) {
    return;
  }
  if (isBattlerInactiveOrFlag4000(D_80148648[0xA]) != 0 &&
      selectionSetupEnabled() == 0 && selectionSetupEnabledB() == 0) {
    return;
  }
  if (*D_801EB4D8 == D_80148648[0xA]) {
    activatePanelTask();
  }
  if ((*D_801EB4D8 & 0x80) != 0) {
    activatePanelTask();
  }
}
