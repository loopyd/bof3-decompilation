#include "bof3/core/slus_internal.h"

/* @behavior Adds a signed delta to a halfword field that saturates into
 *   0..999:
 *   positive delta: store the sum, cap at 999, report 999 - field
 *   negative delta: store the sum, floor at 0, report -field
 *   zero delta:     report 0
 * A field already at 999 (growing) or 0 (shrinking) is left alone and reports
 * 0. The delta only selects the branch through its s16 truncation; the sum
 * uses the whole argument. Both saturation arms report the amount the cap
 * absorbed, sign-extended through the s16 return.
 *
 * The field is written through a plain u16 view: the original's post-store
 * tests read the stored sum from the register and its zeroing store lets the
 * `negu` for the return value take the branch delay slot, neither of which a
 * volatile pointee allows.
 * @source 0x801654F4
 * @status exact
 * @match 100.00
 * @residual none
 */
s16 adjustHalfwordWith999Cap(u16* target, s32 value) {
  s32 signed_value;
  u16 current;

  current = *target;
  signed_value = (s32)(s16)value;

  if (signed_value > 0) {
    if (current == 999) {
      return 0;
    }
    *target = (u16)(current + value);
    if (*target < 1000) {
      return 0;
    }
    *target = 999;
    return 999 - current;
  }

  if (signed_value < 0) {
    if (current == 0) {
      return 0;
    }
    *target = (u16)(current + value);
    if ((s16)*target >= 0) {
      return 0;
    }
    *target = 0;
    return -current;
  }

  return 0;
}
