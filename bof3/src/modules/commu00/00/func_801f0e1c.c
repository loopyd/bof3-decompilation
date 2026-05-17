#include "internal.h"

/* does: applies the type-6 record-specific task-slot variant.
 * @source: 0x801f0e1c
 */
void func_801f0e1c(u8 task_index, u8 record_kind_index) {
  volatile Commu00TaskSlot* task_slot;
  u8                        variant_index;
  u16                       label_id;

  variant_index = COMMU00_VARIANT_ROTATION[record_kind_index];

  if (variant_index == 0u) {
    task_slot = COMMU00_SCRATCH_SLOT;
    task_slot->active = 5u;
    task_slot = COMMU00_SCRATCH_SLOT;
    task_slot->variant_arg_0 = 1u;
    task_slot->variant_arg_1 = 1u;
    label_id = 0xcau;
  } else {
    COMMU00_SCRATCH_SLOT->active = 1u;
    label_id = (u16)((u16)variant_index | 0xc000u);
  }

  commu00_task_slot(task_index)->label_id = label_id;
}
