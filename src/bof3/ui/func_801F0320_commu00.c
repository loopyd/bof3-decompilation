#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0320
 * @behavior Walks the sixty stride-9 local weight records at 0x801F2706 in
 *           lockstep with the stride-8 active-record table: when a record slot
 *           is active its kind byte selects one of the two container-local byte
 *           totals -- kind 10 adds the weight byte to the 0x801455C5 total and
 *           kind 11 adds it to the 0x801455C6 total.
 * @status partial
 * @match 79.55
 * @residual Live asm-diff 35/44 instructions (79.55%), 172 vs 176 bytes, first
 * difference +0x0008: the two hoisted kind literals, the whole loop body, both
 * kind arms, the strength-reduced weight pointer and its 540-byte bound
 * reproduce the original. The residual is the pre-loop accumulator pair: the
 * original materialises the 0x801455C5 symbol address into a temporary and
 * copies it into the loop register (`lui v0 / addiu v0,21957 / move a2,v0`,
 * then `addiu a3,a2,1`), this spelling folds the symbol address into a2 with no
 * copy, so the block is one instruction short and every later position shifts.
 * The named kind constants at the top are retained because they put the two
 * hoisted literals in the original order (the earlier spelling was 34/44 with
 * the first difference at +0x0000). A constant-address spelling
 * (`PSX_PTR(u8, 0x801455C5u)`) reaches 39/44 at 176/176 bytes but is declined:
 * it emits `lui/ori` instead of the original's `lui/addiu` symbol
 * materialisation and folds the second accumulator into its own `lui/ori` pair,
 * so its size match comes from a form the original bytes disprove.
 * Residual class: pre-loop invariant-address copy. Smallest missing evidence:
 * the clean-C spelling that keeps the invariant address temporary separate from
 * the loop-resident register in this compiler profile.
 */
void func_801F0320(void) {
  u8* total_a;
  u8* total_b;
  s32 woff;
  s32 offset;
  u8  kind;
  s32 match_a;
  s32 match_b;

  match_a = 10;
  match_b = 11;
  total_a = &D_801455C5;
  total_b = total_a + 1;

  *total_a = 0;
  D_801455C6 = 0;

  offset = 0;
  for (woff = 0; woff < 540; woff += 9) {
    if (((volatile u8*)activeRecordBytes)[offset] != 0) {
      kind = D_801455C9[offset];

      if (kind == match_a) {
        *total_a = *total_a + D_801F2706[woff];
      } else if (kind == match_b) {
        *total_b = *total_b + D_801F2706[woff];
      }
    }

    offset += 8;
  }
}
