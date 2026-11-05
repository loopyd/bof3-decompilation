#include "bof3/bof3.h"

/* @source 0x801F2D30
 * @behavior Loads the work record from scratchpad pointer-slot 0x1F800044,
 *           decrements its unsigned halfword countdown at +0x2C, adds
 *           0x400000 to word +0x3C, subtracts six from word +0x64 and adds
 *           six to word +0x68. On countdown expiry, rearms it to 24 and
 *           increments dispatch byte +0x01. Takes no arguments and returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementCountdownAdjustWorkWordsRearm24World02Area08213_801F2D30(void) {
  u8* work;
  u16 countdown;

  work = SPAD_PTR_SLOT(u8, 0x44);
  countdown = *(u16*)(work + 0x2C);
  countdown = (u16)(countdown - 1);
  *(u16*)(work + 0x2C) = countdown;
  *(u32*)(work + 0x3C) += 0x400000;
  *(u32*)(work + 0x64) -= 6;
  *(u32*)(work + 0x68) += 6;
  if (countdown == 0) {
    work[1]++;
    *(u16*)(work + 0x2C) = 0x18;
  }
}
