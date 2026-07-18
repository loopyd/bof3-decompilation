#include "internal.h"

/* @behavior Finds value in four 16-byte palette maps and stores its row and
 * column in scratchpad bytes zero and one.
 * @source 0x80196B20
 */
u8 func_80196B20(u8 value) {
  volatile u8* scratch;
  u8           row;
  u8           column;

  scratch = VPTR(u8, 0x1f800000u);
  for (row = 0u; row < 4u; row++) {
    for (column = 0u; column < 0x10u; column++) {
      if (D_80145D54[row][column] == value) {
        scratch[0] = row;
        scratch[1] = column;
        return value;
      }
    }
  }
  return 0xffu;
}
