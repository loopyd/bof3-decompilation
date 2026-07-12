#include "internal.h"

/* @behavior applies the type-13 record-specific task-slot variant.
 * @source 0x801f1254
 */
void func_801f1254(u8 task_index) {
  COMMU00_SCRATCH_SLOT->active = 1u;
  commu00_task_slot(task_index)->label_id = 0xc00au;
}
