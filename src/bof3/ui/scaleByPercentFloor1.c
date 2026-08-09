#include "bof3/ui/shop00_internal.h"

/* @behavior returns (arg0 * (arg1 & 0xFFFF)) / 100, floored to 1; both call
 *           sites pass a record byte as the percent argument.
 * @source 0x801D3654
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u32 scaleByPercentFloor1(u32 arg0, u32 arg1) {
  u32 var_v0;

  var_v0 = (arg0 * (arg1 & 0xFFFF)) / 100;
  if (var_v0 == 0) {
    var_v0 = 1;
  }
  return var_v0;
}
