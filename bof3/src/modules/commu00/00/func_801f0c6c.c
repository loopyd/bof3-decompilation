#include "internal.h"

/* does: applies the type-4 record-specific task-slot variant.
 * @source: 0x801f0c6c
 */
void func_801f0c6c(u8 task_index, u8 record_kind_index) {
  const volatile u8* commu00_state;
  u16                label_id;

  commu00_state = (const volatile u8*)0x80140000u;
  BOF3_COMMU00_SCRATCH_SLOT->active = 1u;

  if (((commu00_state[0x55c4u] == 7u) &&
       (commu00_state[((u32)record_kind_index * 8u) + 0x57a9u] != 0u)) ||
      ((commu00_state[0x55c3u] == 10u) &&
       (commu00_state[((u32)record_kind_index * 8u) + 0x57a9u] == 0u))) {
    label_id = 0xccu;
  } else {
    label_id =
        (u16)(commu00_state[((u32)record_kind_index * 8u) + 0x57a9u] + 0xc8u);
  }

  commu00_task_slot(task_index)->label_id = label_id;
}
