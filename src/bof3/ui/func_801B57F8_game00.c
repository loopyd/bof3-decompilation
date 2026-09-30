#include "bof3/ui/game00_internal.h"

/*
 * @behavior Gate predicate over the shared entry-0 frontend/world bytes.
 *           Returns 0 while either world flag word 0x80146258 / 0x8014625A
 *           carries one of its low three bits set. Otherwise it returns 1 when
 *           the live record count byte 0x80146254 is below 2, when the byte
 *           0x80145FD1 is neither 1 nor 6, or when that count is not 3, and
 *           with the count equal to 3 it returns 1 only when the byte
 *           0x80146111 is 1 or 6.
 * @source 0x801B57F8
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The named constant local is load-bearing: both lookup chains compare against
 * the same materialized 1, reproducing the original's single `li a1,1` in the
 * mode-test delay slot plus the separate return-value materialization. Spelling
 * the literal 1 inline instead folds the returns into the branch delay slots
 * and reallocates that constant (live 29/35, same 140 bytes). */
s32 func_801B57F8(void) {
  u32 mode;
  u8 flag;
  s32 one;

  if (((D_80146258 | D_8014625A) & 7) != 0) {
    return 0;
  }
  mode = D_80146254;
  if (mode < 2) {
    return 1;
  }
  one = 1;
  flag = D_80145FD1;
  if (flag != one && flag != 6) {
    return 1;
  }
  if (mode != 3) {
    return 1;
  }
  flag = D_80146111;
  if (flag == one || flag == 6) {
    return 1;
  }
  return 0;
}
