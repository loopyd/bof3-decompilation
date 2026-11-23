#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F2D38
 * @behavior Overlay counter step: reads the shared halfword counter at
 *           0x8014932A, stores the constant 2 into the shared status byte at
 *           0x80149333, then writes the counter back retreated by 0x12. This is
 *           the retreat half of the pair whose advance half is the adjacent
 *           stateCounterAdvance12: the two 0x28-byte
 *           spans are byte-equal except the one `addiu` immediate word, which
 *           reads `0x2442FFEE` here (`addiu $v0, $v0, -0x12`) and `0x24420012`
 *           there. The leaf has no frame and no caller: its only in-image
 *           reference is the word at 0x801F36B0, a slot of the code-pointer run
 *           based at 0x801F36A0 that lists the overlay's six entries in address
 *           order, so its parameter roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCounterRetreat12(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ - 0x12);
}
