#include "bof3/battle/battle15_internal.h"

/* @source 0x8009C8AC
 * @behavior Reports whether the required mask is present on the current
 * battler: rejects an inert current work record (+0xF8), resolves the mask
 * through the mode 0x4 kind record, then requires mode 1, an entry index below
 * 3, and no 0x100 bit before testing the two-level lookup byte table.
 * @status partial
 * @match 75.86
 * @residual 44/58 instructions, size 232 -> 228 bytes, byte-match DIFFER; first mismatch
 * +0x0014: the original keeps the mode 0x4 result in $v0 (`and v0,v0,a0`, `li v0,1`) and
 * reloads D_80146375 into $v1, while this candidate puts the result in $v1 and reloads the
 * mode into $v0; the two downstream layout consequences are the 0x100 test's inverted branch
 * plus trailing `j` (original `bnez v0,<epilogue>; move v0,zero`) and the final
 * `sltu v0,zero,v0` return (original `bnez`/`li v0,1`/`move v0,zero`). About 25 clean-C
 * variants (declaration initializers, u8/u16/u32/u32-parameter result and mask widths, arm
 * inversion, assignment-in-condition, kind-record pointer/temporary, inline index reads,
 * operand-order swaps) all held the same +0x0014 frontier; only the best coherent candidate
 * was retained. Next untried rung: per-object profile/compiler search (bin/flag-search,
 * bin/compiler-variants) or a bounded permuter run; both are opt-in and outside this lane.
 */
u8 func_8009C8AC(u16 required_mask) {
  u16 mask = required_mask;
  u16 result;
  u32 index;

  if (FIELD_REF(s16, g_BtlCurrentBattler, 0xF8u) == 0) {
    return 0;
  }
  if (D_80146375 == 4) {
    result = D_801CA71C[D_801463C0].mask & required_mask;
    if (result != 0) {
      return 1;
    }
  } else {
    result = 1;
  }
  if (D_80146375 != result) {
    return 0;
  }
  /* Non-volatile view of the same byte: the original loads it once and uses the
   * zero-extended value directly in the 0x140-stride index, which the declared
   * volatile read cannot reproduce (it forces `lbu` + `andi`). */
  index = *(u8*)&D_80146374;
  if (index >= 3) {
    return 0;
  }
  if ((mask & 0x100) != 0) {
    return 0;
  }
  if ((D_801C90EB[D_80145F12[index * 320] * 24] & mask) != 0) {
    return 1;
  }
  return 0;
}
