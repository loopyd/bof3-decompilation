#include "bof3/core/slus_internal.h"

/* @behavior Modifies a bounded byte counter by a signed delta:
 *   positive delta: increment counter, cap at 6
 *   negative delta: decrement counter, floor at 0
 *   zero delta:     set counter to 7 (special max)
 * Returns 1 if the counter value changed, 0 otherwise.
 * @source 0x801655F4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 func_801655F4(u8* counter, s32 delta) {
  s32 signed_delta;

  signed_delta = (s32)(s8)delta;

  if (signed_delta > 0) {
    if (*counter >= 6) {
      return 0;
    }
    *counter = (u8)(*counter + delta);
    if (*counter >= 7) {
      *counter = 6;
    }
    return 1;
  }

  if (signed_delta < 0) {
    if (*counter >= 6) {
      return 0;
    }
    *counter = (u8)(*counter + delta);
    if ((s8)*counter < 0) {
      *counter = 0;
    }
    return 1;
  }

  if (*counter >= 7) {
    return 0;
  }
  *counter = 7;
  return 1;
}
