#include "bof3/bof3.h"

/* @source 0x801F9420
 * @behavior Two-value average on the engine's 0x1000-unit rotation scale: it
 * masks both of its arguments to their low twelve bits in place (0xFFF, one
 * full turn), then orders the two masked values with an unsigned comparison so
 * the smaller one stays in the first argument register and the larger one in
 * the second - the swap stashes the smaller value in the scratch register in
 * the branch delay slot. The difference of the ordered halves, taken as signed,
 * decides the result: below 0x800 the two values are less than half a turn
 * apart and it returns the logical half of their sum, while at or above 0x800
 * it returns half of their sum - shifted arithmetically and formed inline from
 * the same two masked halves - plus 0x800, truncated to 16 bits, which is the
 * circular mean that passes through the wrap point. The 0xFFFF masks on the two
 * ordered halves are deliberate: they make the sum and the difference 16-bit
 * values, so the two arms genuinely differ in signedness. It is a leaf - it
 * makes no call, reads and writes no memory, keeps no callee-saved register and
 * has no frame. Its only call site in this overlay is func_801F8D98, which
 * walks the 0x1F halfword values at offset 0x300 of its object, averages each
 * value with its predecessor and stores each result as the first halfword of
 * the stride-0x18 record array at offset 0x14. The same 0x50 bytes appear at
 * 0x801F8F40 in emi/scenario/scena07/00 and at 0x801F35B0 in
 * emi/world02/area088/13, differing only in the address of their internal
 * jump; every overlay owns its own copy.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 average12BitValuesScenarioScena0800_801F9420(u32 arg0, u32 arg1) {
  u32 t;
  s32 d;

  arg0 &= 0xFFF;
  arg1 &= 0xFFF;
  if (arg1 < arg0) {
    t = arg0;
    arg0 = arg1;
    arg1 = t;
  }
  d = (arg1 & 0xFFFF) - (arg0 & 0xFFFF);
  if (d < 0x800) {
    return ((arg1 & 0xFFFF) + (arg0 & 0xFFFF)) >> 1;
  }
  return (u16)((((s32)((arg1 & 0xFFFF) + (arg0 & 0xFFFF))) >> 1) + 0x800);
}
