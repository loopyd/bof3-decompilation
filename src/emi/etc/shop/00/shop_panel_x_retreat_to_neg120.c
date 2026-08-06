#include "internal.h"

/* @source 0x801E4278
 * @behavior subtracts 0x20 from the panel task x position, clamps to min -120, and
 *         clears state when reached.
 */
void shop_panel_x_retreat_to_neg120(void) {
  PanelTask* task_root;
  u16        next_val;
  s16        clamp_val;

  task_root = D_80148648;
  next_val = (u16)(task_root->x - 0x20);
  task_root->x = next_val;
  if ((s16)next_val < -120) {
    clamp_val = -120;
    task_root->x = (u16)clamp_val;
    task_root->state = 0;
  }
}
