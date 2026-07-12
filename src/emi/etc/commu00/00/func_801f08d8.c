#include "internal.h"

/* @behavior seeds one record-driven task slot, dispatches its type-specific setup,
 * and advances the per-record-kind variant rotation.
 * @source 0x801f08d8 FUN_801f08d8
 */
void func_801f08d8(u8 source_index, u8 task_index) {
  const volatile Commu00ActiveRecord* source_record;
  volatile Commu00TaskSlot*           task_slot;
  u8                                  record_kind;
  u8                                  record_kind_index;
  u8                                  variant_index;
  u8                                  handler_kind;
  u32                                 variant_offset;
  u16                                 palette_id;

  source_record = commu00_active_record(source_index);
  task_slot = commu00_task_slot(task_index);
  COMMU00_SCRATCH_SLOT = task_slot;

  task_slot->source_index = source_index;
  record_kind = source_record->kind;
  record_kind_index = (u8)(record_kind - 1u);
  task_slot->mode = 6u;
  task_slot->variant_state = 0u;
  task_slot->state = 0u;

  variant_index = COMMU00_VARIANT_ROTATION[record_kind_index];
  variant_offset = ((u32)record_kind_index * 9u) + ((u32)variant_index * 3u);
  task_slot->field_34 = 0;
  task_slot->field_36 = (s16)(u16)COMMU00_RECORD_VARIANTS[variant_offset + 0u];
  task_slot->field_38 = 0;
  task_slot->field_3a = (s16)(u16)COMMU00_RECORD_VARIANTS[variant_offset + 1u];
  task_slot->field_3e =
      (s16)commu00_pack_slot_anchor(task_slot->field_34, task_slot->field_38);
  task_slot->resource_id = COMMU00_RECORD_VARIANTS[variant_offset + 2u];
  task_slot->reset_flag = 0u;

  palette_id = COMMU00_SLOT_PALETTE_TABLE[(u32)(task_slot->source_index % 6u)];
  commu00_apply_slot_palette(palette_id);
  commu00_prime_slot_resource(task_slot->resource_id);

  handler_kind = COMMU00_RECORD_KIND_TABLE[(u32)record_kind_index * 8u];
  switch (handler_kind) {
    case 0:
      func_801f0bf4(task_index);
      break;

    case 4:
      func_801f0c6c(task_index, record_kind_index);
      break;

    case 5:
      func_801f0d3c(task_index, record_kind_index);
      break;

    case 6:
      func_801f0e1c(task_index, record_kind_index);
      break;

    case 7:
      func_801f0ec8(task_index);
      break;

    case 8:
      func_801f0f08(source_index, task_index, record_kind_index);
      break;

    case 9:
      func_801f0fbc(source_index, task_index, record_kind_index);
      break;

    case 10:
      func_801f1064(task_index, record_kind_index);
      break;

    case 11:
      func_801f1110(task_index, record_kind_index);
      break;

    case 12:
      func_801f1204(task_index, record_kind_index);
      break;

    case 13:
      func_801f1254(task_index);
      break;

    default:
      break;
  }

  COMMU00_VARIANT_ROTATION[record_kind_index] += 1u;
}
