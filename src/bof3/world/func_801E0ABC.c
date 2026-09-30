#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E0ABC
 * @behavior Reports the AREA030 summary value of one slot amount: when the
 * 16-bit amount has reached the slot record's limit byte at +0x1F the record
 * value at +0x22 is reported unchanged; below the limit that value is scaled
 * by the amount's tenths of the limit and divided back down by ten.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 44/44 instructions, 176 bytes, live byte match.
 * The inverted early return (`amount >= limit`) is what places the scaled path
 * in the branch fall-through; the signed division keeps the original
 * break 7/break 6 trap sequence through the recorded per-object
 * `-Wa,--expand-div` profile in config/compiler/object-flags.cmake.
 */
s32 func_801E0ABC(s32 arg0, s32 arg1) {
  u8 idx = arg0 & 0xff;
  u16 amount = arg1 & 0xffff;

  if (amount >= D_801E2AB8[idx].limit) {
    return D_801E2AB8[idx].value;
  }
  return (u16)(D_801E2AB8[idx].value *
               (amount * 10 / D_801E2AB8[idx].limit) / 10);
}
