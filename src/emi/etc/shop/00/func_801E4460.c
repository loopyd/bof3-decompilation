#include "internal.h"

/* @source 0x801E4460
 * @behavior advances the panel task field at offset 6 by 0x10, clamps to max 0xF0, and
 *         clears state when reached.
 */
void func_801E4460(void) {
  PanelTask* task_root;

  task_root = D_80148648;
  if ((s16)(*(u16*)((u8*)task_root + 6) =
                  (u16)(*(u16*)((u8*)task_root + 6) + 0x10)) >=
      0xF1) {
    *(u16*)((u8*)task_root + 6) = 0xF0;
    task_root->state = 0;
  }
}
