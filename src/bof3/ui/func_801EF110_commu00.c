#include "bof3/ui/commu00_internal.h"

/* @source 0x801EF110
 * @behavior Advances the signed counter byte at 0x801455C2 by the
 *           record-weighted tick delta: reads the shared counter word at
 *           0x8014502C and its snapshot at 0x801455B4, keeps the low 16 bits of
 *           their unsigned difference divided by five as the tick count, and
 *           returns immediately when that count is zero; otherwise it walks the
 *           sixty stride-8 active-record entries at 0x801455C8/0x801455C9 and
 *           sums the container-local byte at 0x801F2705 plus three, at stride
 *           nine, over the present entries whose kind byte equals nine. It then
 *           adds the low 16 bits of that sum minus twice the supplied
 *           active-record count, multiplied by the tick count, to the
 *           sign-extended 0x801455C2 byte and stores the result clamped to
 *           0..99 back into 0x801455C2, then republishes the shared counter word
 *           as the new snapshot.
 * @status partial
 * @match 93.85
 * @residual Live asm-diff 61/65 instructions (93.85%), 260 vs 260 bytes, first
 * difference +0x0040: the whole division/guard, all eighty loop instructions
 * (both stride tables, the weight temporary and its +3, the accumulator) and
 * the multiply reproduce the original instruction-for-instruction including
 * registers (tick in v1, offset a0, weight offset a1, total a2, kind constant
 * a3, penalty t0, value v1, result a0). The residual is two block-local
 * ordering classes: the clamp's branch layout (the original tests `value < 0`
 * first with `bltz` and jumps over the else arm; this spelling emits `bgez`
 * plus a `nop` delay slot) and the placement of the loop-invariant `li a3,9`
 * inside the loop preheader (the original orders it between the total and
 * weight-offset inits, this spelling at the end). Measured clean-C spellings
 * for the clamp chain/ternary/three-store/nested and for the preheader
 * init/declaration order (32 variants including the `weight = table[..] + 3`
 * temporary that fixed the accumulation order and the tick register) either
 * reproduced the same two classes or dropped the already-exact loop and tail
 * registers, so the candidate is retained as the best measured shape.
 * Smallest missing evidence: the clean-C spelling that yields the original's
 * clamp branch polarity/block order and preheader constant placement in this
 * compiler profile.
 */
void func_801EF110(u8 active_count) {
  u32 tick;
  s32 weight;
  s32 penalty;
  s32 offset;
  s32 weight_offset;
  s32 total;
  s32 value;
  s32 clamped;

  tick = (D_8014502C - D_801455B4) / 5;
  if ((u16)tick == 0) {
    return;
  }

  penalty = active_count * 2;
  total = 0;
  weight_offset = 0;
  offset = 0;
  do {
    if (((volatile u8*)activeRecordBytes)[offset] != 0 && D_801455C9[offset] == 9) {
      weight = D_801F2705[weight_offset] + 3;
      total += weight;
    }
    offset += 8;
    weight_offset += 9;
  } while (offset < 480);

  value = D_801455C2 + ((s16)total - penalty) * (u16)tick;
  clamped = value >= 100 ? 99 : value;
  if (value < 0) {
    clamped = 0;
  }
  D_801455C2 = clamped;
  D_801455B4 = D_8014502C;
}
