#include "bof3/ui/commu00_internal.h"

/* @source 0x801F1110
 * @behavior marks the scratch task active and stores the variant-selected label
 *           word on the indexed task: 0xC009 for rotation byte 0, 0xC00B or
 *           0xD4 for rotation byte 1 depending on the shared EXE flag-bank
 *           probe, and 0xD5 for every other rotation byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F1110(u8 task_index, u8 record_kind_index) {
  u8 variant;

  commu00ScratchTask->active = 1;
  variant = D_801F2928[record_kind_index & 0xFF];

  if (variant == 0) {
    taskLabelWords[(task_index & 0xFF) * 76] = 0xC009;
  } else if (variant == 1) {
    if (func_8015B5D4((u32)D_80144F28, 0x92) != 0) {
      taskLabelWords[(task_index & 0xFF) * 76] = 0xC00B;
    } else {
      taskLabelWords[(task_index & 0xFF) * 76] = 0xD4;
    }
  } else {
    taskLabelWords[(task_index & 0xFF) * 76] = 0xD5;
  }
}
