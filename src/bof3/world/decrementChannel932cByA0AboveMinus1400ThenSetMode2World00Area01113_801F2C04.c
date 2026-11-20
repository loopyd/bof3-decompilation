#include "bof3/bof3.h"

extern s16 D_8014932C;
extern u8 D_80149333;

/* @source 0x801F2C04
 * @behavior Overlay step on the shared channel pair the area019 twins at
 * 0x801F2C04/0x801F2C2C also write: it reads the signed 16-bit channel at
 * 0x8014932C once, and while that value is above the -0x1400 floor subtracts
 * 0xA0 from it and stores the result back (slti against -0x13FF, so the test is
 * the upper bound `> -0x1400`), then unconditionally publishes mode 2 into the
 * shared mode byte at 0x80149333. The channel read is copied before the test
 * because the stored value is the one just read. Leaf with no arguments and no
 * return value; no word of this payload calls or points at the entry, so the
 * argument and return roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementChannel932cByA0AboveMinus1400ThenSetMode2World00Area01113_801F2C04(
    void) {
  s16 channel;

  channel = D_8014932C;
  if (channel > -0x1400) {
    D_8014932C = channel - 0xA0;
  }
  D_80149333 = 2;
}
