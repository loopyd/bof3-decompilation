#include "internal.h"

/* @source 0x801E3204
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x84, and
 *         clears state when reached.
 */
void func_801E3204(void) {
  PanelTask* task_root;
  u16        next_val;

  task_root = D_80148648;
  next_val = (u16)(task_root->x - 0x20);
  task_root->x = next_val;
  if ((s16)next_val < 0x84) {
    task_root->x = 0x84;
    task_root->state = 0;
  }
}
