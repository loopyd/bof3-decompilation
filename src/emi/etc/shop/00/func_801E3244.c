#include "internal.h"
#include "bof3/ui/panel_task.h"

/* @source 0x801E3244
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x46, and
 *         clears state when reached.
 */
void func_801E3244(void) {
  PanelTask* task_root;
  u16        next_val;

  task_root = D_80148648;
  next_val = (u16)(task_root->x - 0x20);
  task_root->x = next_val;
  if ((s16)next_val < 0x46) {
    task_root->x = 0x46;
    task_root->state = 0;
  }
}
