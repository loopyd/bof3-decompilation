#include "bof3/bof3.h"

/* @source 0x800C3254
 * @behavior Converts the 16.16 fixed-point value in the first argument to whole
 * units: returns that value >> 16, plus one when its low halfword is non-zero and
 * the direction word `arg1` is non-negative. The overlay handler at 0x800C3284
 * passes the signed product of a per-mode 16-bit scale-table entry (the table
 * inside the func_800C34D4 boundary) and the 16.16 work word +0x40, then adds the
 * result into the 16-bit accumulators at work +0x2E and +0x30; calls 2 and 3 pass
 * `value << 16` with 0x10000 and use the whole value back, confirming the 16.16
 * reading of the first argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 fixedPointToUnitsBossBoss05516_800C3254(s32 value, s32 arg1) {
  if (arg1 >= 0 && (u16)value != 0) {
    s32 whole = value >> 16;

    value &= 0xFFFF;
    value |= (whole + 1) << 16;
  }
  return value >> 16;
}
