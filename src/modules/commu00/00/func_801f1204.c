#include "internal.h"

/* @behavior applies the type-12 record-specific task-slot variant.
 * @source 0x801f1204
 */
void func_801f1204(u8 task_index, u8 record_kind_index) {
  COMMU00_SCRATCH_SLOT->active = 1u;
  commu00_task_slot(task_index)->label_id =
      (u16)(COMMU00_VARIANT_ROTATION[record_kind_index] - -0xc005u);
}
