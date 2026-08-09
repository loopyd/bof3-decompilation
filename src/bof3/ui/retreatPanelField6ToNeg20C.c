#include "bof3/ui/shop00_internal.h"

/* @source 0x801E2B14
 * @behavior subtracts 0x10 from the panel task field at offset 6, clamps to min -0x14, and
 *         clears state when reached.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void retreatPanelField6ToNeg20C(void) {
  PanelTask* task_root;
  u16        next_val;

  task_root = D_80148648;
  next_val = (u16)((*(u16*)((u8*)task_root + 6)) - 0x10);
  *(u16*)((u8*)task_root + 6) = next_val;
  if ((s16)next_val >= -0x13) {
    *(s16*)((u8*)task_root + 6) = -0x14;
    task_root->state = 0;
  }
}
