#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1064
 * @behavior configures the scratch task from a variant byte and stores its task label
 * @status review-pending
 */
void func_801F1064(u8 task_index, u8 record_kind_index) {
  if (variantRotation[record_kind_index & 0xFF] != 0) {
    volatile s8 *active_scratch;
    u32 narrow_task_index;
    u32 offset;

    active_scratch = SPAD_PTR_SLOT(volatile s8, 0x44);
    active_scratch[6] = 1;
    narrow_task_index = task_index & 0xFF;
    offset = narrow_task_index * 76;
    FIELD_REF(u16, taskLabelWords, offset * 2) =
        (u16)(variantRotation[record_kind_index & 0xFF] + 0xCF);
  } else {
    volatile s8 *status_scratch;
    volatile s32 *value_scratch;
    u32 narrow_task_index;
    u32 offset;

    status_scratch = SPAD_PTR_SLOT(volatile s8, 0x44);
    status_scratch[6] = 5;
    value_scratch = SPAD_PTR_SLOT(volatile s32, 0x44);
    value_scratch[6] = 6;
    value_scratch[7] = 0;
    narrow_task_index = task_index & 0xFF;
    offset = narrow_task_index * 76;
    FIELD_REF(u16, taskLabelWords, offset * 2) = 0xCB;
  }
}
