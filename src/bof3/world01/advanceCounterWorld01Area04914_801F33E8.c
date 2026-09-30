#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F33E8
 * @behavior Reads the 16-bit overlay counter D_8014932A, writes the constant
 *           0x02 into the area status byte D_80149333, and stores the
 *           counter advanced by 0x14 back into D_8014932A.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounterWorld01Area04914_801F33E8(void) {
  u16 count;

  count = D_8014932A;
  D_80149333 = 0x02;
  D_8014932A = (u16)(count + 0x14);
}
