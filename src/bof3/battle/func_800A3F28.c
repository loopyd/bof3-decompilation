#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A3F28
 * @behavior Marks the active battle record with flag 0x200 and clears its low status bit.
 * @status partial
 * @match 88.57
 * @residual entry v0/v1 allocation is reversed and a redundant selector andi
 * adds four bytes; width/direct-update variants regressed and one bounded
 * permuter run found no exact candidate.
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
    barrier();
    D_80145FB0[D_80146394].status &= 0xFE;
  } else {
    D_801EB72C[index - 3].flags |= 0x200;
    barrier();
    D_801EB72C[D_80146394 - 3].status &= 0xFE;
  }
}
