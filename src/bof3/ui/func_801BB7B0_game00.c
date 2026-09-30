#include "bof3/ui/game00_internal.h"

/* @source 0x801BB7B0
 * @behavior Maps the low nibble of the scratchpad byte at arg0 to a small
 *           code: 5 for 2, 2 for 1 or 15, and 4 for every other value.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801BB7B0(u8 arg0) {
  s32 kind;

  kind = D_1F800000[arg0] & 0x0F;

  switch (kind) {
  case 1:
  case 15:
    return 2;
  case 2:
    return 5;
  case 3:
    return 4;
  default:
    return 4;
  }
}
