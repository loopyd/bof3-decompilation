#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @source 0x80096F78
 * NOTE: byte-match blocked by the same `jr ra` epilogue scheduler reorg as
 * func_80096AB0. The original battle/15 object used `-fno-schedule-insns`;
 * restoring that per-target profile makes this match (exact-duplicate of
 * func_800983C4).
 */
void func_80096F78(void) {
  u32 temp = func_801502D0(0x4000);
  func_801DE8C0(2, 0xFF, temp);
  PSX_REF(volatile u8, (u32)&D_801462E4) += 1;
}
