#include "internal.h"

/* @behavior applies the type-5 record-specific task-slot variant.
 * @source 0x801f0d3c
 */
void func_801f0d3c(u8 task_index, u8 record_kind_index) {
  u8  record_kind_flag_1;
  u8  record_kind_flag_2;
  u32 variant_arg_1;

  if (COMMU00_VARIANT_ROTATION[record_kind_index] != 0u) {
    COMMU00_SCRATCH_SLOT->active = 1u;
    commu00_task_slot(task_index)->label_id =
        (u16)(COMMU00_VARIANT_ROTATION[record_kind_index] + 0xd1u);
  } else {
    const volatile u8* record_kind;

    COMMU00_SCRATCH_SLOT->active = 5u;
    commu00_task_slot(task_index)->label_id = 0xc008u;
    COMMU00_SCRATCH_SLOT->variant_arg_0 = 0u;
    record_kind = &COMMU00_RECORD_KIND_TABLE[((u32)record_kind_index * 8u)];
    record_kind_flag_1 = record_kind[1];
    record_kind_flag_2 = record_kind[2];
    variant_arg_1 =
        ((u32)record_kind_flag_1 * 2u) + ((u32)record_kind_flag_2 + 0x11u);
    COMMU00_SCRATCH_SLOT->variant_arg_1 = variant_arg_1;
  }
}
