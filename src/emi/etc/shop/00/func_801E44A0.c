#include "internal.h"

/* @source 0x801E44A0
 * @behavior subtracts 0x10 from panel-task field 6; on underflow, clamps it to 0x80 and clears state.
 */
void func_801E44A0(void) {
  PanelTask* task_root;
  s16        next_val;
  task_root = D_80148648;
  next_val = (s16)(task_root->field_06 - (0x10));
  task_root->field_06 = (u16)next_val;
  if (next_val < (0x80)) {
    next_val = (0x80);
    task_root->field_06 = (u16)next_val;
    task_root->state = 0;
  }
}
