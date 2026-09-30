#include "bof3/battle/battle15_internal.h"

/* @source 0x800A250C
 * @behavior Runs the selection-input pass func_800A3F28, then, when the
 *   selection roll func_800A3B6C returns zero for (D_80146374, D_80146394,
 *   0x80), applies the 0x8 input mask to the byte D_80146394 through
 *   func_800A31E0.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The two pre-call argument reads use non-volatile views of the declared
 * volatile bytes: the volatile publication would place the 0x80 argument
 * constant before them, leaving a nop in the closing jal delay slot. The
 * post-call read keeps the volatile symbol so the byte is re-read after the
 * call rather than kept in a callee-saved register. */
void runSelectionPassThenApplyMask8WhenRollClear(void) {
  func_800A3F28();
  if ((u8)func_800A3B6C(*(u8 *)&D_80146374, *(u8 *)&D_80146394, 0x80) == 0) {
    func_800A31E0(D_80146394, 8);
  }
}
