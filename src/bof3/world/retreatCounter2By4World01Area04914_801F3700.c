#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F3700
 * @behavior Reads the halfword counter at 0x8014932A, writes 2 to the status
 *           byte at 0x80149333, then stores the counter minus 4 modulo 65536.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter2By4World01Area04914_801F3700(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ - 0x04);
}
