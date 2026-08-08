#include "internal.h"

/* @source 0x801999B8
 * @behavior advances the panel task field_06 by 0x10, clamps to max 0x28, and
 *         clears state when the clamp is reached.
 */
void func_801999B8(void) {
  PanelTask* task;
  u16        next;

  task = D_80148648;
  next = task->field_06 + 0x10;
  task->field_06 = next;
  if ((s16)next >= 0x29) {
    task->field_06 = 0x28;
    task->state = 0;
  }
}
