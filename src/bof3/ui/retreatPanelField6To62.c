#include "bof3/ui/shop00_internal.h"
#include "base/compiler.h"

/* @source 0x801E2D1C
 * @behavior subtracts 0x10 from the panel task field at offset 6, raises values below 0x3E, and
 *         clears state when reached.
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void retreatPanelField6To62(void) {
  PanelTask* task_root;

  u16 next_val;

  task_root = D_80148648;
  next_val = (u16)((*(volatile u16*)((u8*)task_root + 6)) - 0x10);
  *(volatile u16*)((u8*)task_root + 6) = next_val;
  if ((s16)next_val < 0x3E) {
    *(volatile u16*)((u8*)task_root + 6) = 0x3E;
    task_root->state = 0;
  }
}
