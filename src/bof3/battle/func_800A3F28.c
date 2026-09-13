#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A3F28
 * @behavior Marks the active battle record with flag 0x200 and clears its low status bit.
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void func_800A3F28(void) {
  volatile u16 *flags;
  u16 value;
  u32 index;

  flags = &D_801462E8;
  value = *flags;
  index = D_80146394;
  *flags = value | 0x2000;
  if (index < 3) {
    D_80145FB0[index].flags |= 0x200;
    D_80145FB0[D_80146394].status &= 0xFE;
  } else {
    D_801EB72C[index - 3].flags |= 0x200;
    D_801EB72C[D_80146394 - 3].status &= 0xFE;
  }
}
