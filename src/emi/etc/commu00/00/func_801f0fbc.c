#include "internal.h"

/* @behavior applies the type-9 record-specific task-slot variant.
 * @source 0x801f0fbc
 */
void func_801f0fbc(u8 source_index, u8 task_index, u8 record_kind_index) {
  u32 record_state;

  (void)record_kind_index;

  if ((record_state = commu00_active_record(source_index)->record_state) ==
      0u) {
    COMMU00_SCRATCH_SLOT->active = 1u;
    commu00_task_slot(task_index)->label_id = 0x51u;
    return;
  }

  if (record_state == 1u) {
    COMMU00_SCRATCH_SLOT->unk_00 = 0u;
    return;
  }

  COMMU00_SCRATCH_SLOT->active = 1u;
  commu00_task_slot(task_index)->label_id = 0xc004u;
}
