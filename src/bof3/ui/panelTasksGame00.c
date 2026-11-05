#include "bof3/ui/game00_internal.h"
#include "ui/panel_task.h"
#include "shared/ui/panel_task.inc"

/* @source 0x801999B8
 * @behavior advances the panel task field_06 by 0x10, clamps to max 0x28, and
 *         clears state when the clamp is reached.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_FIELD6(advancePanelField6To40, 0x28)

/* @source 0x8019982C
 * @behavior Adds 32 to 16-bit panel x; signed results above 17 clamp to 17 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo17, 17)

/* @source 0x801996FC
 * @behavior Adds 32 to 16-bit panel x; signed results above 320 clamp to 320 and clear state.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo320, 320)

/* @source 0x80199638
 * @behavior advances the panel x coordinate by 32 and stops at 80.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_ADVANCE_X(advancePanelXTo80, 0x50)

/* @source 0x80199678
 * @behavior submits the current panel x and field-six coordinates to the panel helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_APPLY_POSITION(applyPanelPosition)

/* @source 0x801995F8
 * @behavior retreats panel x with the template clamp behavior.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X(retreatPanelXToNeg170, -0xAA)

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

/* @source 0x8019973C
 * @behavior retreats panel x by 32 and clamps it to 152.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X_CLAMP(retreatPanelXTo152, 0x98)

/* @source 0x801997EC
 * @behavior retreats panel x with the template clamp behavior.
 * @status exact
 * @match 100.00
 * @residual none
 */
PANEL_RETREAT_X(retreatPanelXToNeg170B, -0xAA)
