#include "bof3/battle/battle03_internal.h"

/* @behavior Sums the halfword at +0x96 of the D_80145E90 local work records
 * selected by the localBattlerCount record count, divides that 16-bit total by the same
 * count and returns half of the sum of the quotient and the D_801EC2EE halfword.
 * @source 0x801DCCB0
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 func_801DCCB0(void) {
  s32 total;
  u32 count;
  u32 limit;
  u32 index;
  u8  slot;
  s32 divisor;

  /* Plain views retain the original two count loads and one final halfword
   * load without imposing volatile ordering on the intervening arithmetic. */
  total = 0;
  index = 0u;
  count = *(u8*)&localBattlerCount;
  if (count != 0u) {
    limit = count;
    do {
      slot = (u8)index;
      total += *(u16*)&D_80145F26[((slot << 2) + slot) << 6];
      index += 1u;
    } while ((u8)index < limit);
  }

  divisor = *(u8*)&localBattlerCount;
  return ((u32)((u16)total / divisor) + *(u16*)&D_801EC2EE) >> 1;
}
