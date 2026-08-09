#include "bof3/ui/shop00_internal.h"

/* @source 0x801E37B4
 * @behavior subtracts 0x20 from the panel task x position, clamps to max 0x4B, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void retreatPanelXTo75(void) {
  PanelTask* task_root;
  u16        next_val;

  task_root = D_80148648;
  next_val = (u16)(task_root->x - 0x20);
  task_root->x = next_val;
  if ((s16)next_val < 0x4B) {
    task_root->x = 0x4B;
    task_root->state = 0;
  }
}
