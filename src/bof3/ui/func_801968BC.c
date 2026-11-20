#include "bof3/ui/game00_internal.h"

/* @behavior Scans four 16-byte palette maps for an 0xff range selected by mode
 * and records its row and column in scratchpad bytes zero and one. Mode 0
 * returns the flat 16-per-row index of the run minus 0x40, modes 1 to 4 accept
 * the run only at the mode's alignment (1, 2, 4 and 8 entries wide, mode 1
 * covering the rest of the row) and return the packed run code of that
 * alignment; a run that does not reach the accepted end or an unlisted mode
 * continues the scan, and the scan reports 0xff when no run matches.
 * @source 0x801968BC
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801968BC(u8 mode) {
  u8 row;
  u8 column;
  u8 end;
  s32 run_limit;

  for (row = 0u; row < 4u; row++) {
    for (column = 0u; column < 0x10u; column++) {
      if (D_80145D54[row][column] == 0xffu) {
        D_1F800000[0] = row;
        D_1F800000[1] = column;
        switch (mode) {
          case 0u:
            return (u8)(column + row * 16u - 0x40u);
          case 1u:
            if (column != 0u)
              break;
            end = column;
            run_limit = 0x0f;
            while (end <= run_limit && D_80145D54[row][end] == 0xffu)
              end++;
            if (end == 0x10u) {
              return (u8)(row + (column >> 4) + 0x1cu);
            }
            break;
          case 2u:
            if ((column & 1u) != 0u)
              break;
            run_limit = column + 1;
            end = column;
            while (end <= run_limit && D_80145D54[row][end] == 0xffu)
              end++;
            if (end == column + 2) {
              return (u8)(row * 8u + (column >> 1) - 0x20u);
            }
            break;
          case 3u:
            if ((column & 3u) != 0u)
              break;
            run_limit = column + 3;
            end = column;
            while (end <= run_limit && D_80145D54[row][end] == 0xffu)
              end++;
            if (end == column + 4) {
              return (u8)(row * 4u + (column >> 2) + 0x70u);
            }
            break;
          case 4u:
            if ((column & 7u) != 0u)
              break;
            run_limit = column + 7;
            end = column;
            while (end <= run_limit && D_80145D54[row][end] == 0xffu)
              end++;
            if (end == column + 8) {
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
