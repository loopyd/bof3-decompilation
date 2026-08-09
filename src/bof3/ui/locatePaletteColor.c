#include "bof3/ui/game00_internal.h"

/* @behavior Finds value in four 16-byte palette maps and stores its row and
 * column in scratchpad bytes zero and one.
 * @source 0x80196B20
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 locatePaletteColor(u8 value) {
  u8 row;
  u8 column;

  for (row = 0u; row < 4u; row++) {
    for (column = 0u; column < 0x10u; column++) {
      if (D_80145D54[row][column] == value) {
        D_1F800000[0] = row;
        D_1F800000[1] = column;
        return value;
      }
    }
  }
  return 0xffu;
}
