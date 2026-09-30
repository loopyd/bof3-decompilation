#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0D3C
 * @behavior selects a label for the indexed task slot from the record-kind rotation
 * byte and seeds the slot's variant arguments from the record-kind table pair.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F0D3C(u8 task_index, u8 record_kind_index) {
  s32 pair_shift;
  s32 pair_value;

  if (variantRotation[record_kind_index & 0xFF] != 0) {
    commu00ScratchTask->active = 1;
    taskLabelWords[(task_index & 0xFF) * 76] =
        variantRotation[record_kind_index & 0xFF] + 0xD1;
  } else {
    commu00ScratchTask->active = 5;
    taskLabelWords[(task_index & 0xFF) * 76] = 0xC008;
    commu00ScratchTask->variant_arg_0 = 0;
    pair_shift = D_801457A9[(record_kind_index & 0xFF) * 8] << 1;
    pair_value = D_801457AA[(record_kind_index & 0xFF) * 8] + 0x11;
    commu00ScratchTask->variant_arg_1 = pair_shift + pair_value;
  }
}
