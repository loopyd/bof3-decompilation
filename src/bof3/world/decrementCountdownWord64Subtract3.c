#include "bof3/bof3.h"

/* @source 0x801F2D90
 * @behavior Loads the work record from scratchpad pointer-slot 0x1F800044,
 *           decrements its unsigned halfword countdown at +0x2C, adds three to
 *           the word at +0x64 and subtracts three from the word at +0x68. When
 *           the decremented countdown reaches zero it rearms it to 0x10 and
 *           increments dispatch byte +0x01. Takes no arguments and returns
 *           nothing. Entry index 4 of the twelve-pointer step table at
 *           0x801F5734, which the overlay entry point 0x801F2C04 indexes with
 *           work byte +0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementCountdownWord64Subtract3(void) {
  u8* work;
  u16 countdown;

  work = SPAD_PTR_SLOT(u8, 0x44);
  countdown = *(u16*)(work + 0x2C);
  countdown = (u16)(countdown - 1);
  *(u16*)(work + 0x2C) = countdown;
  *(u32*)(work + 0x64) += 3;
  *(u32*)(work + 0x68) -= 3;
  if (countdown == 0) {
    work[1]++;
    *(u16*)(work + 0x2C) = 0x10;
  }
}
