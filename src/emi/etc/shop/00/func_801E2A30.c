#include "internal.h"

/* @source 0x801E2A30
 * @behavior retreats panel field six by 16 and clamps it to -20.
 */
void func_801E2A30(void) {
  PanelTask* task_root;
  s16        next_val;
  task_root = D_80148648;
  next_val = (s16)(task_root->field_06 - (0x10));
  task_root->field_06 = (u16)next_val;
  if (next_val < (-0x14)) {
    next_val = (-0x14);
    task_root->field_06 = (u16)next_val;
    task_root->state = 0;
  }
}
