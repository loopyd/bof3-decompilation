#include "internal.h"

/* does: seeds one template-driven task slot using the current source index and
 * task slot index.
 * @source: 0x801f0718 FUN_801f0718
 */
void func_801f0718(u8 source_index, u8 task_index) {
  volatile Commu00TaskSlot* task_slot;
  u32                       template_offset;
  u16                       palette_id;

  task_slot = commu00_task_slot(task_index);
  BOF3_COMMU00_SCRATCH_SLOT = task_slot;
  task_slot->active = 1u;

  template_offset = (u32)task_index * 3u;
  if (BOF3_COMMU00_TASK_TEMPLATE_TABLE[template_offset + 2u] == 0u) {
    task_slot->mode = 6u;
    task_slot->variant_state = 0u;
    task_slot->state = 0u;
  } else {
    task_slot->mode = 0u;
    task_slot->variant_state = 2u;
    task_slot->state = 1u;
  }

  task_slot->field_34 = 0;
  task_slot->field_36 =
      (s16)(u16)BOF3_COMMU00_TASK_TEMPLATE_TABLE[template_offset + 0u];
  task_slot->field_38 = 0;
  task_slot->field_3a =
      (s16)(u16)BOF3_COMMU00_TASK_TEMPLATE_TABLE[template_offset + 1u];
  task_slot->field_3e =
      (s16)commu00_pack_slot_anchor(task_slot->field_34, task_slot->field_38);
  task_slot->source_index = source_index;

  palette_id =
      BOF3_COMMU00_SLOT_PALETTE_TABLE[(u32)(task_slot->source_index % 6u)];
  commu00_apply_slot_palette(palette_id);
  commu00_prime_slot_resource(task_slot->resource_id);
}
