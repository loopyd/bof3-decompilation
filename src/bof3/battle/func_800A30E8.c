#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A30E8
 * @behavior Computes a negated, table-scaled battle value for a selector: the
 *   battle globals D_801EC312 (+100) and D_801463C0 (times 20) scale the
 *   percentage byte of the record at D_801CA71B, and the selector byte picks
 *   the element byte of the 320-byte-stride table at 0x80145F34 (indices below
 *   three) or of the 280-byte-stride table at 0x801EB6E4 (larger indices),
 *   which selects the s16 modifier table at 0x800B497C; the signed result is
 *   divided by 10000, negated and returned as a sign-extended s16.
 * @status partial
 * @match 54.84
 * @residual 34/62 instructions, 248 bytes against the original's 248. The two
 *   global loads now come first and independently, as the original's
 *   `lui`/`lhu` pairs do, and the range size is exact; the live diff is the
 *   entry register assignment (the candidate loads D_801EC312 into $v1 and
 *   D_801463C0 into $a0, the original the opposite way) plus one scheduler
 *   move: the candidate hoists the D_801CA71B percent-byte load above the
 *   amount*100 scaling, the original finishes that scaling first. Every later
 *   difference only renames $v0/$v1 to follow that entry assignment. The split
 *   initialiser is what removes the old `lhu`+`nop` load-delay pair: with
 *   `D_801EC312 + 100` as one initialiser the second global load cannot be
 *   independent and the row is 252 bytes. Measured repair shapes (matched of
 *   62, bytes): split initialiser 34, 248 (retained); declared-uninitialised
 *   locals assigned in the body 34, 248; record declared first 34, 248;
 *   record*20 before the increment 34, 248; amount fully scaled before the
 *   record scale 34, 248 (measured twice from an identical body); s32 locals
 *   34, 248; `+ 100` as a body assignment 31, 252; record*20 folded into the
 *   initialiser 34, 256; `(amount + 100) * 100` 31, 252. Earlier history:
 *   statement order, signed/unsigned locals, a u16 source local and a fully
 *   pre-scaled amount all kept the old entry
 *   schedule, so the residual is allocator/scheduling, not source shape. The
 *   `(index - 3) * 280` offset local is required: inlining the subscript lets
 *   gcc fold -840 into the %lo displacement and drop the original's
 *   `addiu $v1,$v1,-3`.
 */
s32 func_800A30E8(u32 unused, u8 selector)
{
  u32 amount = D_801EC312;
  u32 record = D_801463C0;
  u32 base;
  u32 index;
  u32 offset;
  s32 scale;

  (void)unused;
  amount = amount + 100;
  record = record * 20;
  amount = amount * 100;
  base = D_801CA71B[record];
  base = (base * amount) / 100;
  index = selector;
  if (index < 3) {
    index = D_80145F34[index * 320];
  } else {
    offset = (index - 3) * 280;
    index = D_801EB6E4[offset];
  }
  scale = ((s32)base * D_800B497C[index]) / 10000;
  return (s16)-scale;
}
