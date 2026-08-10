#include "bof3/ui/game00_internal.h"

/* @behavior Returns whether two scratchpad bytes differ when both high nibbles
 * identify type 0x20; otherwise returns zero.
 * @source 0x801BB81C
 * @status partial
 * @match 30.00
 * @residual First mismatch +0x8: original addu at,at,a0 versus current
 * addu at,a0,at; canonical/all historical profiles and one 60s permuter exhausted.
 */
s32 func_801BB81C(u8 arg0, u8 arg1) {
  u8  first;
  u8  second;
  u8  type;
  s32 result;

  first = SPAD_REF(u8, arg0);
  type = first & 0xF0;
  result = 0;
  if (type == 0x20) {
    second = SPAD_REF(u8, arg1);
    if ((second & 0xF0) == type) {
      result = second != first;
    }
  }
  return result;
}
