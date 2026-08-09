#include "bof3/world/area00813_internal.h"

/* @behavior clears the 16-bit count, sets state byte 2, returns the
 * constant 2.
 * @source 0x801F455C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 resetCounter1(void) {
  s32 state = 2;

  counter1 = 0;
  D_80149333 = (u8)state;
  return state;
}
