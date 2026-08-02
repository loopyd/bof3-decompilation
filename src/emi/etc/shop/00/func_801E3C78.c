#include "internal.h"

/* @source 0x801E3C78
 * @behavior advances the panel task x position by 0x20, clamps to max 0x28, and
 *         clears state when reached.
 */
void func_801E3C78(void) {
  PanelTask* task_root;
  u16        next_val;

  task_root = D_80148648;
  next_val = (u16)(task_root->x + 0x20);
  task_root->x = next_val;
  if ((s16)next_val >= 0x29) {
    task_root->x = 0x28;
    task_root->state = 0;
  }
}
