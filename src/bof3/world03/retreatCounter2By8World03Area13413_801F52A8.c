#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F52A8
 * @behavior Overlay counter step: marks the shared counter active by storing 2
 * into the area status byte at 0x80149333, then retreats the halfword counter
 * at 0x8014932A by 0x08. It is the byte-shape mirror of the adjacent
 * retreatCounter1By2World03Area13413_801F51BC and
 * advanceCounter2By8World03Area13413_801F5280 in this same payload: same
 * load-first idiom and same counter halfword as the advance, with the
 * immediate negated.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter2By8World03Area13413_801F52A8(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ - 0x08);
}
