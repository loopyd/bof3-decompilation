#include "internal.h"

/* @behavior UNKNOWN: verify the arithmetic seed against target-qualified assembly.
 * @source 0x801D3654
 */
s32 func_801D3654(s32 arg0, s32 arg1) {
  s32 var_v0;

  var_v0 = (arg0 * (arg1 & 0xFFFF)) / 100;
  if (var_v0 == 0) {
    var_v0 = 1;
  }
  return var_v0;
}
