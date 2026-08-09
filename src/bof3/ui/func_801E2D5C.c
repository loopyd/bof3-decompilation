#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2D5C
 * @behavior advances panel field 6 by 0x10, clamps it to 54 times byte 0xA plus 0x3E,
 *         and clears state when the limit is exceeded.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801E2D5C(void) {
  PanelTask* task_root;
  u16 next_val;
  u32 index;
  s32 signed_next;
  s32 limit;

  task_root = D_80148648;
  next_val = (u16)(task_root->field_06 + 0x10);
  index = *((u8*)task_root + 0xA);
  task_root->field_06 = next_val;
  signed_next = (s16)next_val;
  limit = (index * 54) + 0x3E;
  if (limit < signed_next) {
    task_root->field_06 = (u16)limit;
    task_root->state = 0;
  }
}
