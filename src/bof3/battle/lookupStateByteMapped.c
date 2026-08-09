#include "bof3/battle/battle03_internal.h"

/* @source 0x801DD7AC
 * @behavior returns one indexed state byte, mapping value seven to zero.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 lookupStateByteMapped(s32 arg0) {
  u8 var = D_80181B10[arg0 & 0xFF];

  if ((var & 0xFF) == 7) {
    var = 0;
  }
  return var;
}
