#include "internal.h"

/* @source 0x801E2B54
 * @behavior advances the panel task at offset 6 by 0x10, clamps to max 0x26, and
 *         clears state when reached.
 */
void shop_panel_field6_advance_to_38_2(void) {
  PanelTask* task_root;

  task_root = D_80148648;
  if ((s16)(*(u16*)((u8*)task_root + 6) =
                  (u16)(*(u16*)((u8*)task_root + 6) + 0x10)) >=
      0x27) {
    *(u16*)((u8*)task_root + 6) = 0x26;
    task_root->state = 0;
  }
}
