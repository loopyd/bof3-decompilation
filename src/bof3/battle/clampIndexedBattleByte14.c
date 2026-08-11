#include "bof3/battle/battle15_internal.h"

/* @source 0x800A39B4
 * @behavior Adds a signed delta to byte 0x14 of the selected battle record and clamps it to [-25, 50].
 * @status review-pending
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
