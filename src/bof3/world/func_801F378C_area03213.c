#include "bof3/world/area03213_internal.h"

/* @source 0x801F378C
 * @behavior Queries the cursor level state through func_801C29A0(8): when the
 * query reports non-zero it clears byte 4 of the scratch cursor record at
 * 0x1F800044, otherwise it consumes two units of the shared countdown word at
 * 0x122 of the current work record and advances the cursor record 0x34
 * accumulator by its 0x0C step.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 29/29 instructions, 116 -> 116 bytes, exact on the third live
 * diff. Two clean-C shapes settled it: the countdown arm is the if-body so the
 * fall-through stays in line and the single-store arm is reached by the `bnez`,
 * and reading the cursor pointer before the countdown store lets the scheduler
 * place `lui`/`lw` in the `lhu` load-delay slot with the record in `$a0`, which
 * the later 0x34/0x0C pair then reuses.
 */
void func_801F378C(void) {
  u8* record;

  if (!(u8)func_801C29A0(8)) {
    record = D_1F800044;
    *(u16*)(D_80146250 + 0x122) -= 2;
    *(s32*)(record + 0x34) += *(s32*)(record + 0x0c);
  } else {
    D_1F800044[4] = 0;
  }
}
