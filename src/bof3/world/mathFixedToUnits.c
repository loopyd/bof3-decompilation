#include "bof3/bof3.h"

/* @source 0x801F3DCC
 * @behavior Converts the 16.16 fixed-point value in the first argument to whole
 * units: returns that value >> 16, plus one when its low halfword is non-zero and
 * the direction word `arg1` is non-negative. The overlay handler at 0x801F3B98
 * scales a signed 16-bit entry of the work-byte-+0x06 tables at 0x801F43D4 and
 * 0x801F43D6 by the 16.16 work words +0x40 and +0x44 and passes the product with
 * that same work word, adding the returned whole units into the 16-bit
 * accumulators at work +0x2E and +0x30; its other arm converts an entry with
 * `entry << 16` and passes 0x10000, then uses the whole value back, which
 * confirms the 16.16 reading of the first argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 mathFixedToUnits(s32 value, s32 arg1) {
  if (arg1 >= 0 && (u16)value != 0) {
    s32 whole = value >> 16;

    value &= 0xFFFF;
    value |= (whole + 1) << 16;
  }
  return value >> 16;
}
