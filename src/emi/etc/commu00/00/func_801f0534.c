#include "internal.h"

/* @behavior rebuilds the active COMMU00 task-slot set from the 0x3c-entry source
 * record table, with special label paths for kinds `0`, `10`, and `11`.
 * @source 0x801f0534 FUN_801f0534
 */
void func_801f0534(void) {
  u8                                  source_index;
  u8                                  task_index;
  const volatile Commu00ActiveRecord* source_record;
  volatile Commu00TaskSlot*           task_slot;
  u8                                  record_kind;
  u16                                 label_id;
  u16                                 random_value;

  source_index = 0u;
  while (source_index < 8u) {
    COMMU00_VARIANT_ROTATION[source_index] = 0u;
    source_index += 1u;
  }

  source_index = 0u;
  task_index = 0u;
  while (source_index < 0x3cu) {
    source_record = commu00_active_record(source_index);
    if (source_record->active != 0u) {
      record_kind = source_record->kind;
      if (record_kind == 0u) {
        func_801f0bf4(task_index);
        func_801f0718(source_index, task_index);
      } else if (record_kind < 9u) {
        func_801f08d8(source_index, task_index);
      } else if (record_kind == 10u) {
        label_id = 0xcdu;
        if (COMMU00_WORLD_STATE < 0xb6u) {
          random_value = (u16)(game_random_u16() & 7u);
          if (random_value > 5u) {
            random_value = (u16)(random_value - 5u);
          }
          label_id = (u16)(random_value + 0xb6u);
        }

        task_slot = commu00_task_slot(task_index);
        task_slot->label_id = label_id;
        func_801f0718(source_index, task_index);
      } else if (record_kind == 11u) {
        if ((COMMU00_WORLD_STATE == 0xafu) || (COMMU00_WORLD_STATE == 0xb2u) ||
            (COMMU00_WORLD_STATE == 0xb5u) || (COMMU00_WORLD_STATE == 0xb9u)) {
          label_id = 0xceu;
        } else {
          random_value = (u16)(game_random_u16() & 7u);
          if (random_value > 5u) {
            random_value = (u16)(random_value - 5u);
          }
          label_id = (u16)(random_value + 0xbcu);
        }

        task_slot = commu00_task_slot(task_index);
        task_slot->label_id = label_id;
        func_801f0718(source_index, task_index);
      }

      task_index += 1u;
    }

    source_index += 1u;
  }

  while (task_index < 0x14u) {
    commu00_task_slot(task_index)->active = 0u;
    task_index += 1u;
  }
}
