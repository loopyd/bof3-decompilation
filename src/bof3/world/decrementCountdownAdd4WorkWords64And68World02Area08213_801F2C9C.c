#include "bof3/bof3.h"

/* @source 0x801F2C9C
 * @behavior Loads the work record from scratchpad pointer-slot 0x1F800044,
 *           decrements its unsigned halfword countdown at +0x2C, and adds four
 *           to each word at +0x64 and +0x68. When the decremented countdown
 *           reaches zero, rearms it to 16 and increments dispatch byte +0x01.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementCountdownAdd4WorkWords64And68World02Area08213_801F2C9C(void) {
  u8* work;
  u16 countdown;

  work = SPAD_PTR_SLOT(u8, 0x44);
  countdown = *(u16*)(work + 0x2C);
  countdown = (u16)(countdown - 1);
  *(u16*)(work + 0x2C) = countdown;
  *(u32*)(work + 0x64) += 4;
  *(u32*)(work + 0x68) += 4;
  if (countdown == 0) {
    work[1]++;
    *(u16*)(work + 0x2C) = 0x10;
  }
}
