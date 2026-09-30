#include "bof3/bof3.h"

extern volatile u16 D_8014932A;
extern volatile u8 D_80149333;

/* @source 0x801F2F48
 * @behavior Overlay counter step: marks the shared counter active by storing 2
 * into the area status byte at 0x80149333, then advances the 16-bit counter at
 * 0x8014932A by 0x0A.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounter2By10World00Area00713_801F2F48(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ + 0x0A);
}
