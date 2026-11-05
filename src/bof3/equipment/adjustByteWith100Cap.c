#include "bof3/core/slus_internal.h"

/* @behavior Adds a delta to a byte field that saturates at 100. A field that
 * already reads 100 is left alone and reports no change. Otherwise the sum is
 * stored truncated to a byte, a negative stored byte is cleared, and a stored
 * byte above 100 is capped. Returns 1 when the field was updated and 0 when the
 * field already sat at the cap. The delta is not sign-shaped: both directions
 * clamp through the stored byte.
 * @source 0x80165694
 * @status partial
 * @match 68.18
 * @residual 15/22 instructions, 88/88 bytes. Remaining differences are register
 * only: the original keeps the raw byte (a2) and its zero-extended copy (v1)
 * apart, feeds the sum from the raw byte into a fresh v0, and materialises 100
 * in v0. This build's expander emits one shared zero-extend (andi) and the
 * allocator ties the dying delta operand to the sum, so the load lands in v0,
 * the constant in v1 and the sum in a1. Eleven measured clean-C shapes
 * (declaration, cast, mask-local, commutative, narrow-sum, compare-first) did
 * not move it; the RTL dumps in .rtl/.cse/.greg show a single merged
 * zero_extend whose source pseudo dies at that instruction.
 */
s32 adjustByteWith100Cap(volatile u8* target, s32 value) {
  u8 current;
  s32 sum;

  current = *target;
  sum = current + value;
  if ((current & 0xFF) == 100) {
    return 0;
  }
  *target = (u8)sum;
  if ((s8)sum < 0) {
    *target = 0;
  }
  if (*target < 101) {
    return 1;
  }
  *target = 100;
  return 1;
}
