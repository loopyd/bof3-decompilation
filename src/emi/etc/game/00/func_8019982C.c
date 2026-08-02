#include "internal.h"

/* @source 0x8019982C
 * @behavior advances panel x by 17 with the template clamp behavior.
 */
void func_8019982C(void) {
  PanelTask* task_root;
  u16        next_x;
  task_root = D_80148648;
  next_x = (u16)(task_root->x + 32u);
  task_root->x = next_x;
  if ((s16)next_x >= (17) + 1) {
    task_root->x = (17);
    task_root->state = 0u;
  }
}
