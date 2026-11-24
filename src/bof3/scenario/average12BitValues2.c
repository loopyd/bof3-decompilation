#include "bof3/bof3.h"

/* @source 0x801F8F40
 * @behavior Two-value average on the engine's 0x1000-unit rotation scale: it
 * masks both arguments to their low twelve bits in place (0xFFF, one full
 * turn), then orders the two masked values with an unsigned comparison so the
 * smaller one stays in the first argument register and the larger one in the
 * second, stashing the smaller value in the scratch register in the branch
 * delay slot. The signed difference of the two 16-bit halves decides the
 * result: below 0x800 the values are less than half a turn apart and it
 * returns the logical half of their sum, while at or above 0x800 it returns
 * half of their sum, shifted arithmetically, plus 0x800 and truncated to 16
 * bits - the circular mean that passes through the wrap point. The 0xFFFF
 * masks on the ordered halves make the sum and the difference 16-bit values,
 * which is why the two arms differ in signedness. It is a leaf: it makes no
 * call, reads and writes no memory, keeps no callee-saved register and has no
 * frame. Its only reference in this overlay was the jal at 0x801F8B90 inside
 * the span func_801F88B8. The 0x50-byte span was byte-identical to the exact
 * average12BitValuesScenarioScena0800_801F9420 in emi/scenario/scena08/00
 * except for the internal j word at offset 0x3C, whose encoded target moved by
 * exactly the span delta (0x4E0 address, 0x138 encoded): a cmp of the two
 * payload spans reported those two bytes as the only difference.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 average12BitValues2(u32 arg0, u32 arg1) {
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
