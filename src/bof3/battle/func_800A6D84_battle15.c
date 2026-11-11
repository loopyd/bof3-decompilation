#include "bof3/battle/battle15_internal.h"

extern u8 func_800A805C(void); /* @source 0x800A805C */

/* @source 0x800A6D84
 * @behavior Scans the active byte list at 0x801463C4 while the list count
 *   0x801463C7 is non-zero and returns zero when no entry is 0x10 or when the
 *   party counter byte 0x801462F1 is not 3; the first entry that is 0x10 under
 *   that counter value runs func_800A805C and returns its low byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_800A6D84(void)
{
  u8 i;

  i = 0;
  if (D_801463C7 != 0) {
    do {
      if (D_801463C4[i] == 0x10 && D_801462F1 == 3) {
        return func_800A805C();
      }
      i++;
    } while (i < D_801463C7);
  }
  return 0;
}
