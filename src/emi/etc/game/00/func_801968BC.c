#include "internal.h"

/* @behavior Scans four 16-byte palette maps for an 0xff range selected by mode
 * and records its row and column in scratchpad bytes zero and one.
 * @source 0x801968BC
 */
u8 func_801968BC(u8 mode) {
  u8 row;
  u8 column;
  u8 end;

  for (row = 0u; row < 4u; row++) {
    for (column = 0u; column < 0x10u; column++) {
      if (D_80145D54[row][column] == 0xffu) {
        SPAD_REF(volatile u8, 0x00u) = row;
        SPAD_REF(volatile u8, 0x01u) = column;
        switch (mode) {
          case 0u:
            return (u8)(column + row * 16u - 0x40u);
          case 1u:
            if (column != 0u) {
              break;
            }
            end = column;
            while (end < 0x10u && D_80145D54[row][end] == 0xffu) {
              end++;
            }
            if (end == 0x10u) {
              return (u8)(row + (column >> 4) + 0x1cu);
            }
            break;
          case 2u:
            if ((column & 1u) != 0u) {
              break;
            }
            end = column;
            while (end < (u8)(column + 2u) && D_80145D54[row][end] == 0xffu) {
              end++;
            }
            if (end == (u8)(column + 2u)) {
              return (u8)(row * 8u + (column >> 1) - 0x20u);
            }
            break;
          case 3u:
            if ((column & 3u) != 0u) {
              break;
            }
            end = column;
            while (end < (u8)(column + 4u) && D_80145D54[row][end] == 0xffu) {
              end++;
            }
            if (end == (u8)(column + 4u)) {
              return (u8)(row * 4u + (column >> 2) + 0x70u);
            }
            break;
          case 4u:
            if ((column & 7u) != 0u) {
              break;
            }
            end = column;
            while (end < (u8)(column + 8u) && D_80145D54[row][end] == 0xffu) {
              end++;
            }
            if (end == (u8)(column + 8u)) {
              return (u8)(row * 2u + (column >> 3) + 0x38u);
            }
            break;
          default:
            break;
        }
      }
    }
  }
  return 0xffu;
}
