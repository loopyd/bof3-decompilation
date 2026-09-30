#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1064
 * @behavior Configures the scratch commu00 task from the variant byte: while the
 *           variant is nonzero it marks the task active and stores the
 *           variant-derived label word; otherwise it marks the task active,
 *           seeds the variant arguments to 6 and 0, and stores the fixed 0xCB
 *           label word. The label word is addressed by the source index times
 *           76 halfwords.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F1064(u8 task_index, u8 record_kind_index) {
  if (variantRotation[record_kind_index & 0xFF] != 0) {
    commu00ScratchTask->active = 1;
    taskLabelWords[(task_index & 0xFF) * 76] =
        (u16)(variantRotation[record_kind_index & 0xFF] + 0xCF);
  } else {
    commu00ScratchTask->active = 5;
    commu00ScratchTask->variant_arg_0 = 6;
    commu00ScratchTask->variant_arg_1 = 0;
    taskLabelWords[(task_index & 0xFF) * 76] = 0xCB;
  }
}
