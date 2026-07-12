#include "internal.h"

/* @source 0x801d3654 */
s32 func_801d3654(s32 arg0, s32 arg1) {
  s32 var_v0;

  var_v0 = (arg0 * (arg1 & 0xFFFF)) / 100;
  if (var_v0 == 0) {
    var_v0 = 1;
  }
  return var_v0;
}
