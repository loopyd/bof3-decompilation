#include "internal.h"

/* @behavior entry-3 handler of the overlay entry table (D_801F4200):
 * clears D_80147A58 and increments scratch-work byte 0x01.
 * @source 0x801F2DB4
 */
void resetScratchSelectEntry3(void) {
  u8* work;

  work = (u8*)WORLD00_AREA024_SCRATCH_PTR;
  D_80147A58 = 0;
  work[1]++;
}
