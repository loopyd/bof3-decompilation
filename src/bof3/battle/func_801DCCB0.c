#include "bof3/battle/battle03_internal.h"

/* @behavior averages the current local battler `0x96` values and blends the result
 * with the queued halfword at `0x801ec2ee`.
 * @source 0x801DCCB0
 * @status partial
 * @match 35.00
 * @residual non-exact live audit: 14/40 instructions; 160 original bytes versus 140 current.
 */
u32 func_801DCCB0(void) {
  s32 total;
  u32 count;
  u8  index;

  total = 0;
  index = 0u;
  count = BATTLE_GLOBAL_BYTE_62F0;
  if (count != 0u) {
    do {
      total += BATTLE_LOCAL_ABS_HALF_5F26(index);
      index += 1u;
    } while ((u32)index < count);
  }

  return ((((u16)total / BATTLE_GLOBAL_BYTE_62F0) + BATTLE_GLOBAL_HALF_EC2EE) >>
          1);
}
