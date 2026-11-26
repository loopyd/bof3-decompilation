#include "bof3/ui/game00_internal.h"

/* @behavior Returns whether two scratchpad bytes differ when both high nibbles
 *           identify type 0x20, reading both bytes from the scratchpad byte
 *           array D_1F800000 indexed by the unsigned argument; otherwise returns
 *           zero.
 * @source 0x801BB81C
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 compareTypeNibbleBytes(u8 arg0, u8 arg1) {
  u8 first;
  u8 second;
  u8 type;

  first = D_1F800000[arg0];
  type = first & 0xF0;
  if (type == 0x20) {
    second = D_1F800000[arg1];
    if ((second & 0xF0) == type) {
      if (first != second) {
        return 1;
      }
    }
  }
  return 0;
}
