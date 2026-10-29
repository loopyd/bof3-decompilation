#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F4908
 * @behavior Overlay counter step: reads the shared halfword counter at
 * 0x8014932A, stores the constant 2 into the shared status byte at 0x80149333,
 * then writes the counter back retreated by 0x02. It is the load-first mirror
 * of the adjacent advanceCounter2By2World04Area18813_801F48E0 in this same
 * payload: identical idiom on the same counter halfword with the immediate
 * negated. The address appears as a .word entry at 0x801F67BC, between
 * advanceCounter2By2World04Area18813_801F48E0 (0x801F67B8) and
 * setTwoWorld04Area18813_801F4930 (0x801F67C0), in the payload's handler
 * pointer table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter2By2World04Area18813_801F4908(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ - 0x02);
}
