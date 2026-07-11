#include "internal.h"

/* @behavior applies the type-10 record-specific task-slot variant.
 * @source 0x801f1064
 */
void func_801f1064(u8 task_index, u8 record_kind_index) {
  volatile Commu00TaskSlot* task_slot;
  u16                       label_id;

  if (COMMU00_VARIANT_ROTATION[record_kind_index] != 0u) {
    COMMU00_SCRATCH_SLOT->active = 1u;
    label_id = (u16)(COMMU00_VARIANT_ROTATION[record_kind_index] + 0xcfu);
  } else {
    task_slot = COMMU00_SCRATCH_SLOT;
    task_slot->active = 5u;
    task_slot = COMMU00_SCRATCH_SLOT;
    task_slot->variant_arg_0 = 6u;
    task_slot->variant_arg_1 = 0u;
    label_id = 0xcbu;
  }

  commu00_task_slot(task_index)->label_id = label_id;
}
