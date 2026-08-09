#include "bof3/ui/shop00_internal.h"

/* @source 0x801E42B8
 * @behavior advances the panel task x position by 0x20, clamps to max 0x64, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void advancePanelXTo100(void) {
  PanelTask* task_root;
  u16        next_val;

  task_root = D_80148648;
  next_val = (u16)(task_root->x + 0x20);
  task_root->x = next_val;
  if ((s16)next_val >= 0x65) {
    task_root->x = 0x64;
    task_root->state = 0;
  }
}
