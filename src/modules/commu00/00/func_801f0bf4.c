#include "internal.h"

/* @behavior seeds one empty/default task label in the active task-slot table.
 * @source 0x801f0bf4 FUN_801f0bf4
 */
void func_801f0bf4(u8 task_index) {
  volatile Commu00TaskSlot* task_slot;
  u16                       random_value;

  random_value = (u16)(game_random_u16() & 7u);
  if (random_value > 4u) {
    random_value = (u16)(random_value - 4u);
  }

  task_slot = commu00_task_slot(task_index);
  task_slot->active = 1u;
  task_slot->label_id = (u16)(random_value + 0xc2u);
}
