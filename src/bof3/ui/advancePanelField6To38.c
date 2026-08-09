#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2A70
 * @behavior advances the panel task at offset 6 by 0x10, clamps to max 0x26, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void advancePanelField6To38(void) {
  PanelTask* task_root;
  u16        next_val;

  task_root = D_80148648;
  next_val = (u16)(task_root->field_06 + 0x10);
  task_root->field_06 = next_val;
  if ((s16)next_val >= 0x27) {
    task_root->field_06 = 0x26;
    task_root->state = 0;
  }
}
