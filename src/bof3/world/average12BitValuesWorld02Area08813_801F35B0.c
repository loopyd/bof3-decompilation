#include "bof3/bof3.h"

/* @source 0x801F35B0
 * @behavior Two-value average on the engine's 0x1000-unit rotation scale: it
 * masks both arguments to their low twelve bits in place (0xFFF, one full
 * turn), then orders the two masked values with an unsigned comparison so the
 * smaller stays in the first argument register and the larger in the second;
 * the swap stashes the first value in the scratch register in the branch delay
 * slot. The 0xFFFF masks make both ordered halves 16-bit quantities, so the
 * difference is signed and the sum is a 16-bit sum. Below 0x800 the two halves
 * are less than half a turn apart and the result is the logical half of their
 * sum. At or above 0x800 it is the arithmetic half - formed inline from the
 * same two masked halves - plus 0x800, truncated to 16 bits, which is the
 * circular mean that passes through the wrap point. It is a leaf: no call, no
 * memory read or write, no callee-saved register and no frame.
 *
 * The 0x50 bytes are the same body as the exact
 * average12BitValuesScenarioScena0800_801F9420 and the unlifted 0x801F8F40 in
 * emi/scenario/scena07/00: a byte comparison of this span against the scena08
 * payload differs only in the two low bytes of the internal `j` word, which
 * encode the jump target address, so every overlay owns its own copy. The
 * label follows the already-reviewed sibling's role. This target's only
 * in-image reference is the `jal` at 0x801F3200 inside
 * advanceTrailProjectPointsFillHeadingsAndAverageAnglesWorld02Area08813_801F2F28,
 * which passes both angles in a0/a1 and consumes the halfword result.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 average12BitValuesWorld02Area08813_801F35B0(u32 arg0, u32 arg1) {
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
