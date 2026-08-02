#include "internal.h"

/* @source 0x801E31C4
 * @behavior advances panel x toward 320 with the template clamp behavior.
 */
void func_801E31C4(void) {
  PanelTask* task_root;
  u16        next_x;
  task_root = D_80148648;
  next_x = (u16)(task_root->x + 32u);
  task_root->x = next_x;
  if ((s16)next_x >= (320) + 1) {
    task_root->x = (320);
    task_root->state = 0u;
  }
}
