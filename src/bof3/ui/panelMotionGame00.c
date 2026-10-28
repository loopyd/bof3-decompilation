#include "bof3/ui/game00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801995F8
 * @behavior retreats panel x with the template clamp behavior.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X(retreatPanelXToNeg170)

/* @source 0x80199938
 * @behavior retreats panel field six by 16 and clamps it to -20.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_FIELD6(retreatPanelField6ToNeg20, 0x10, -0x14)

/* @source 0x80199978
 * @behavior advances the panel task field_06 by 0x10, clamps to max 0x10, and
 *         clears state when the clamp is reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advancePanelField6To16(void) {
  PanelTask* task;
  u16        next;

  task = D_80148648;
  next = task->field_06 + 0x10;
  task->field_06 = next;
  if ((s16)next >= 0x11) {
    task->field_06 = 0x10;
    task->state = 0;
  }
}
