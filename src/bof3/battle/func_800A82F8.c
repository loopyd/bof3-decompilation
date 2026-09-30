#include "bof3/battle/battle15_internal.h"

/* @source 0x800A82F8
 * @behavior Zeroes the byte accumulator at D_801463B8, then adds the mapped
 *   value of every active list byte into it, once per entry of the active
 *   byte count.
 * @status partial
 * @match 77.78
 * @residual Clean-C partial (21/27 instructions, 104->108 bytes). Loop body,
 *   index masking, table addressing, bound copies and branch chains match.
 *   First difference: preheader schedule. Original loads the count first
 *   (lui $v1 / lbu $v1), computes the accumulator pointer in $v0, then puts
 *   the zero store in the loop-entry branch delay slot and copies the pointer
 *   and count into $a1/$a2. This shape loads the count last, so the assembler
 *   injects an aspsx-style nop before the loop-entry branch and the zero store
 *   is emitted before the branch instead of in its delay slot. Untried next rung: hoisting
 *   the count read out of the loop condition into a register-valued
 *   temporary placed before the pointer statement. Clean-C byte match and
 *   independent review pending.
 */
void func_800A82F8(void)
{
  u8 i;
  u8 *dst;
  u8 *p;

  dst = &D_801463B8;
  *dst = 0;
  p = dst;
  for (i = 0; i < D_801463C7; i++) {
    *p += D_800B4C7C[D_801463C4[i]];
  }
}
