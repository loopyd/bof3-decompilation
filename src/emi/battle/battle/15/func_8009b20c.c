#include "internal.h"

/* @behavior walks the four local panel-task slots rooted at `0x80148330` for task
 * ids `0x10..0x13` and resets each one through the shared task reset helper.
 * @source 0x8009b20c FUN_8009b20c
 */
void __attribute__((noinline)) func_8009b20c(void) {
  u8 panel_task_id;

  panel_task_id = 0x10u;
  do {
    BATTLE_LOCAL_PANEL_TASK_ROOT =
        (volatile u8*)(0x80148330u + ((u32)panel_task_id * 0x24u));
    panel_task_id += 1u;
    battle_reset_local_task_slot();
  } while (panel_task_id < 0x14u);
}
