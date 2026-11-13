#include "bof3/battle/battle15_internal.h"

extern u8 func_800A6D84(void); /* @source 0x800A6D84 */
extern u8 func_800A6CF8(u8 index, u8 value); /* @source 0x800A6CF8 */

extern u8 D_800B4D4C[]; /* @source 0x800B4D4C @kind table */
extern u8 D_800B6F5C; /* @source 0x800B6F5C @kind data */

/* @source 0x800A6C2C
 * @behavior Takes the first of the eleven rows of the three-byte table at
 *   0x800B4D58 whose three entries are each the 0xFF sentinel or present in
 *   the active byte list at 0x801463C4, then copies the row's group byte from
 *   the table at 0x800B4D4C into 0x800B6F5C and returns the row index, or
 *   returns 0xFF when no row is covered. Row six is decided by the guard
 *   func_800A6D84 instead: its non-zero low byte returns, otherwise that row is
 *   skipped and the scan resumes at the next one.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 resolveDragonFormFromGenes(void)
{
  u8 i;
  u8 j;
  u8 guard;

  for (i = 0; i < 11; i++) {
    if (i == 6) {
      guard = func_800A6D84();
      if (guard != 0) {
        return guard;
      }
      i++;
    }
    for (j = 0; j < 3; j++) {
      if (func_800A6CF8(i, j) == 0) {
        break;
      }
    }
    if (j == 3) {
      D_800B6F5C = D_800B4D4C[i];
      return i;
    }
  }
  return 0xFF;
}
