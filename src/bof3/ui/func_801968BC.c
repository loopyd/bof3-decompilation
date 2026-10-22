#include "bof3/ui/game00_internal.h"

/* @behavior Scans four 16-byte palette maps for an 0xff range selected by mode
 * and records its row and column in scratchpad bytes zero and one. Mode 0
 * returns the flat 16-per-row index of the run minus 0x40, modes 1 to 4 accept
 * the run only at the mode's alignment (1, 2, 4 and 8 entries wide, mode 1
 * covering the rest of the row) and return the packed run code of that
 * alignment; a run that does not reach the accepted end or an unlisted mode
 * continues the scan, and the scan reports 0xff when no run matches.
 * @source 0x801968BC
 * @status partial
 * @match 88.24
 * @residual 135 of 153 instructions at the original 612 bytes. The scan, the
 * four accepted-run lengths, the success codes and the scratchpad column/row
 * pair match. Remaining classes: the mode dispatch reads the jump table at
 * 0x80196B20, which the layout places directly after the text, while the
 * original table sits at 0x8019581C in the payload's leading table blob inside
 * the raw `header` segment, so the table base, its register and the dispatch
 * load cannot agree without a reviewed layout that exposes that table (parent
 * owned); the scratchpad pair costs five instructions (`lui v0,0x1f80`,
 * `ori v0,0x1`, `lui v1,0x1f80`, two `sb`) where the original holds `$at` for
 * both symbol-relative stores; and the mode-1 length bound reads signed 0xF
 * while this build folds it to `sltiu`, 0x10. Measured and rejected: named
 * array scratchpad stores with the bound test first (81/153 at 608 bytes),
 * named array stores with the earlier unsigned bounds (45/153 at 724 bytes),
 * the original unsigned `end < 0x10u` bounds with the run test first (73/153 at
 * 740 bytes, frame introduced), masked run-end comparisons (105/153 at 620
 * bytes) and unsigned bound suffixes (81/153 at 608 bytes); the signed and
 * unsigned spellings of the mode-1 bound both measure 135/153. Dropping
 * `volatile` from the two scratchpad stores does fold them to the original's
 * four-instruction `$at` pair, but the run scan's register web then diverges and
 * the candidate measures 81/153 at 608 bytes, the same result as the named-array
 * stores: the store form is not what holds this residual.
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
            while (end <= 0x0fu && D_80145D54[row][end] == 0xffu) {
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
            while (end <= column + 1 && D_80145D54[row][end] == 0xffu) {
              end++;
            }
            if (end == column + 2) {
              return (u8)(row * 8u + (column >> 1) - 0x20u);
            }
            break;
          case 3u:
            if ((column & 3u) != 0u) {
              break;
            }
            end = column;
            while (end <= column + 3 && D_80145D54[row][end] == 0xffu) {
              end++;
            }
            if (end == column + 4) {
              return (u8)(row * 4u + (column >> 2) + 0x70u);
            }
            break;
          case 4u:
            if ((column & 7u) != 0u) {
              break;
            }
            end = column;
            while (end <= column + 7 && D_80145D54[row][end] == 0xffu) {
              end++;
            }
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
