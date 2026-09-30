#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F2D6C
 * @behavior Overlay counter step: marks the shared counter active by storing 2
 * into the area status byte at 0x80149333, then retreats the halfword counter
 * at 0x8014932A by 0x04.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter2By4World03Area13313_801F2D6C(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ - 0x04);
}
