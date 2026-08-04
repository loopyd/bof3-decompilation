#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @source 0x80096F78
 * The original accesses D_801462E4 non-volatile: a volatile store pins the
 * `sb` before the epilogue `lw ra` (program order), while the original
 * scheduler moves it after `lw ra` to cover the load-delay gap before
 * `jr ra` and fills the branch delay slot with `addiu sp`. Exact duplicate
 * of func_800983C4.
 */
void func_80096F78(void) {
  u32 temp = func_801502D0(0x4000);
  func_801DE8C0(2, 0xFF, temp);
  PSX_REF(u8, (u32)&D_801462E4) += 1;
}
