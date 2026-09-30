#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0320
 * @behavior Walks the sixty stride-9 local weight records at 0x801F2706 in
 *           lockstep with the stride-8 active-record table: when a record slot
 *           is active its kind byte selects one of the two container-local byte
 *           totals -- kind 10 adds the weight byte to the 0x801455C5 total and
 *           kind 11 adds it to the 0x801455C6 total.
 * @status partial
 * @match 77.27
 * @residual Live asm-diff 34/44 instructions (77.27%), bytes 172->176 (-4), first difference +0x0000. The complete loop body is instruction-for-instruction identical to the original, including its registers (a2/a3 accumulators, a0 weight pointer, a1 byte offset, t0 final value), the signed `slt` exit test on the strength-reduced weight pointer and the store in the `j` delay slot. The whole residual is the pre-loop block: the original materialises the 0x801455C5 address as `lui v0 / addiu v0,21957 / move a2,v0` (a temp plus a copy into the loop register), which this source emits as a single `lui a2 / addiu a2` pair, so the pre-header is one instruction short and the two hoisted kind constants (10, 11) plus `move a1,zero` are ordered differently. 31 measured clean-C spellings were screened and every one of them either reproduced the single-register form or changed the (already exact) loop: `weight` pointer loops, index loops with `i*8`/`i*9` (kept the counter and `slti`), the table indexed by an `s32` offset (the spelling that produced the exact signed `slt` exit test and the exact loop), symbol-form accumulations (folded `%lo` addressing instead of `a2`/`a3`), accumulator pointers assigned outside vs. inside the loop, `&D_801455C4 + 1` and `total_b = total_a + 1` derivations, declaration initializers, and volatile pointer views (which reached the original's 176-byte size by duplicating the weight-pointer increment into the `j` delay slot -- an unjustified-volatility scheduling trick, rejected). Residual class: pre-loop address materialisation / scheduling, i.e. the original's address survived as a loop invariant register with a copy. Smallest missing evidence: the pass that keeps an invariant temporary separate from the loop-resident register in this compiler profile, or an independent review of the retained candidate. */
void func_801F0320(void) {
  u8* total_a;
  u8* total_b;
  s32 woff;
  s32 offset;
  u8  kind;

  total_a = &D_801455C5;
  total_b = total_a + 1;

  *total_a = 0;
  D_801455C6 = 0;

  offset = 0;
  for (woff = 0; woff < 540; woff += 9) {
    if (((volatile u8*)activeRecordBytes)[offset] != 0) {
      kind = D_801455C9[offset];

      if (kind == 10) {
        *total_a = *total_a + D_801F2706[woff];
      } else if (kind == 11) {
        *total_b = *total_b + D_801F2706[woff];
      }
    }

    offset += 8;
  }
}
