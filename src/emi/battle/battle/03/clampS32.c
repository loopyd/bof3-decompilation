#include "bof3/bof3.h"

/* @source 0x801DD7D8
 * @behavior clamps the middle argument between the other two signed bounds.
 */
s32 clampS32(s32 arg0, s32 arg1, s32 arg2) {
  s32 arg0_is_less;

  arg0_is_less = arg0 < arg2;
  if (arg2 < arg1) {
    return arg0;
  }
  if (arg0_is_less) {
    return arg1;
  }
  return arg2;
}
