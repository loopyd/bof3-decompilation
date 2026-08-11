#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0E1C
 * @behavior activates a task scratch record and writes its variant-dependent label word.
 * @status exact
 * @match 100.00
 */
void func_801F0E1C(u8 task_index, u8 record_kind_index) {
  if (variantRotation[record_kind_index & 0xFF] == 0) {
    commu00ScratchTask->active = 5;
    commu00ScratchTask->variant_arg_0 = 1;
    commu00ScratchTask->variant_arg_1 = 1;
    taskLabelWords[(task_index & 0xFF) * 76] = 0xCA;
  } else {
    commu00ScratchTask->active = 1;
    taskLabelWords[(task_index & 0xFF) * 76] =
        (u16)(variantRotation[record_kind_index & 0xFF] | 0xC000);
  }
}
