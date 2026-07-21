#include "internal.h"
#include "bof3/ui/panel_task.h"

/* @source 0x801E44A0
 * @behavior subtracts 0x10 from the panel task field at offset 6, clamps to max 0x7F (min 0x80), and
 *         clears state when reached.
 */
void func_801E44A0(void) {
  PanelTask* task_root;
  u16        next_val;

  task_root = D_80148648;
  next_val = (u16)((*(volatile u16*)((u8*)task_root + 6)) - 0x10);
  *(volatile u16*)((u8*)task_root + 6) = next_val;
  if ((s16)next_val < 0x80) {
    *(volatile u16*)((u8*)task_root + 6) = 0x80;
    task_root->state = 0;
  }
}
