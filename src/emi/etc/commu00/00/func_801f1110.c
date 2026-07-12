#include "internal.h"

/* @behavior applies the type-11 record-specific task-slot variant.
 * @source 0x801f1110
 */
void func_801f1110(u8 task_index, u8 record_kind_index) {
  volatile Commu00TaskSlot* task_slot;
  u8                        variant_index;
  u16                       label_id;

  task_slot = COMMU00_SCRATCH_SLOT;
  task_slot->active = 1u;
  variant_index = COMMU00_VARIANT_ROTATION[record_kind_index];

  if (variant_index == 0u) {
    label_id = 0xc009u;
  } else if (variant_index == 1u) {
    if (commu00_check_selector_flag((const void*)0x80144f28u, 0x92) == 0) {
      label_id = 0xd4u;
    } else {
      label_id = 0xc00bu;
    }
  } else {
    label_id = 0xd5u;
  }

  commu00_task_slot(task_index)->label_id = label_id;
}
