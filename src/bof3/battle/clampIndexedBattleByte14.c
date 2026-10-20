#include "bof3/battle/battle15_internal.h"

/* @source 0x800A39B4
 * @behavior Adds a signed delta to byte 0x14 of the selected battle record and clamps it to [-25, 50].
 * @status partial
 * @match 86.96
 * @residual instruction-scheduling residual: the prologue stack adjustment
 *   `addiu sp,sp,-8` is emitted first here but sits in the first `bnez` delay
 *   slot in the original, which shifts the branch target and the shared store
 *   block by four bytes (20/23 instructions, 92->92 bytes). Rejected variants:
 *   single-store result local (11/23, 72 bytes), `sum > 0x32` (20/23) and a
 *   nested clamp test (19/23). The 52-profile canonical flag catalog returns
 *   no exact match (best 86.96) and GCC 2.8.1 diverges wholesale (best 50.0).
 */
void clampIndexedBattleByte14(s16 delta, s32 index)
{
  s8 *entry;
  s8 current;
  s32 sum;

  entry = (s8 *)D_801463A0 + (index & 0xFF);
  current = entry[0x14];
  sum = current + delta;
  if (sum >= 0x33) {
    entry[0x14] = 0x32;
  } else if (sum < -0x19) {
    entry[0x14] = -0x19;
  } else {
    entry[0x14] = delta + current;
  }
}
