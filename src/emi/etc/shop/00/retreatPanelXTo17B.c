#include "internal.h"

/* @source 0x801E42F8
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x11, and
 *         clears state when reached.
 */
void retreatPanelXTo17B(void) {
  PanelTask* task_root;
  u16        next_val;

  task_root = D_80148648;
  next_val = (u16)(task_root->x - 0x20);
  task_root->x = next_val;
  if ((s16)next_val < 0x11) {
    task_root->x = 0x11;
    task_root->state = 0;
  }
}
