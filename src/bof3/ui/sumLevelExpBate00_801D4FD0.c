#include "bof3/context.h"
#include "data/equipment.h"

/* @source 0x801CB8DC @kind data */
extern LevelObject D_801CB8DC[][99];

/* @source 0x801D4FD0
 * @behavior sums the first count experience entries for the selected character
 *           in the shared level table; returns -1 for counts of 100 or greater.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 sumLevelExpBate00_801D4FD0(u8 row, u8 count) {
  u8 index;
  s32 total;

  if (count >= 100) {
    return -1;
  }

  total = 0;
  for (index = 0; index < count; index++) {
    total += D_801CB8DC[row][index].exp;
  }
  return total;
}
