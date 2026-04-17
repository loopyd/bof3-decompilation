#include "internal.h"

/* does: applies the type-7 record-specific task-slot variant.
 * @source: 0x801f0ec8
 */
void func_801f0ec8(u8 task_index) {
  BOF3_COMMU00_SCRATCH_SLOT->active = 1u;
  commu00_task_slot(task_index)->label_id = 0xc003u;
}
