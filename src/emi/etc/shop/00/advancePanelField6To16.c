#include "internal.h"

/* @source 0x801E2988
 * @behavior advances the panel task field_06 by 0x10, clamps to max 0x10, and
 *         clears state when the clamp is reached.
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
