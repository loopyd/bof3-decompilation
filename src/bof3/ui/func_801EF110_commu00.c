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
 * @status exact
 * @match 100.00
 * @residual none
 * Two clean-C shapes carry the last four instructions and are kept for that
 * reason. The kind literal is a named local assigned between the accumulator
 * and offset initializations, so the loop-invariant compare constant stays
 * where the original issues it (between those two inits) instead of being
 * hoisted behind them. The clamp tests the non-negative guard with the value
 * arm nested inside it, which yields the original's branch-first `bltz` to the
 * out-of-line zero arm instead of a `bgez` with an inline fall-through arm.
 */
void func_801EF110(u8 active_count) {
  u32 tick;
  s32 weight;
  s32 match_kind;
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
  match_kind = 9;
  weight_offset = 0;
  offset = 0;
  do {
    if (((volatile u8*)activeRecordBytes)[offset] != 0 && D_801455C9[offset] == match_kind) {
      weight = D_801F2705[weight_offset] + 3;
      total += weight;
    }
    offset += 8;
    weight_offset += 9;
  } while (offset < 480);

  value = D_801455C2 + ((s16)total - penalty) * (u16)tick;
  if (value >= 0) {
    if (value < 100) {
      clamped = value;
    } else {
      clamped = 99;
    }
  } else {
    clamped = 0;
  }
  D_801455C2 = clamped;
  D_801455B4 = D_8014502C;
}
