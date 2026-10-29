#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F5280
 * @behavior Overlay counter step: marks the shared counter active by storing 2
 * into the area status byte at 0x80149333, then advances the halfword counter
 * at 0x8014932A by 0x08.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounter2By8World03Area13413_801F5280(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ + 0x08);
}
