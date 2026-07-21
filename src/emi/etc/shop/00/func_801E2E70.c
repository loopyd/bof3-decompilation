#include "internal.h"
#include "bof3/ui/panel_task.h"

/* @source 0x801E2E70
 * @behavior advances the panel task x position by 0x20, clamps to max 0xF, and
 *         clears state when reached.
 */
void func_801E2E70(void) {
  PanelTask* task_root;
  u16            next_val;

  task_root = D_80148648;
  next_val = (u16)(task_root->x + 0x20);
  task_root->x = next_val;
  if ((s16)next_val >= 0x10) {
    task_root->x = 0xF;
    task_root->state = 0;
  }
}
