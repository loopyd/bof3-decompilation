#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F2D14
 * @behavior Overlay counter step: reads the shared halfword counter at
 * 0x8014932A, marks the counter active by storing 2 into the area status byte
 * at 0x80149333, then retreats the halfword counter by 0x0F. It is the same
 * load-first idiom as the exact retreatCounter2By8World03Area13413_801F52A8 in
 * another world overlay, with this overlay's own step immediate, so the loaded
 * value is stepped and stored after the status byte is published.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter2By15World02Area08113_801F2D14(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ - 0x0F);
}
