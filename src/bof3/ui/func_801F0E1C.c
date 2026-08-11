#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0E1C
 * @behavior activates a task scratch record and writes its variant-dependent label word.
 * @status partial
 * @match 81.40
 * @residual same-size allocator/address-materialization mismatch begins at the branch delay slot.
 */
void func_801F0E1C(u8 task_index, u8 record_kind_index) {
  if (variantRotation[record_kind_index & 0xFF] == 0) {
    PSX_REF(volatile Commu00TaskSlot *, SPAD_ADDRESS(0x44u))->active = 5;
    PSX_REF(volatile Commu00TaskSlot *, SPAD_ADDRESS(0x44u))->variant_arg_0 = 1;
    PSX_REF(volatile Commu00TaskSlot *, SPAD_ADDRESS(0x44u))->variant_arg_1 = 1;
    taskLabelWords[(task_index & 0xFF) * 76] = 0xCA;
  } else {
    PSX_REF(volatile Commu00TaskSlot *, SPAD_ADDRESS(0x44u))->active = 1;
    taskLabelWords[(task_index & 0xFF) * 76] =
        (u16)(variantRotation[record_kind_index & 0xFF] | 0xC000);
  }
}
