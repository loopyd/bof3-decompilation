#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F2D60
 * @behavior Overlay counter step: reads the shared halfword counter at
 *           0x8014932A, stores the constant 2 into the shared status byte at
 *           0x80149333, then writes the counter back advanced by 0x12. This is
 *           the advance half of the pair whose retreat half is the adjacent
 *           func_801F2D38: same load-first idiom and same counter halfword with
 *           the immediate negated.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounter2By18World00Area02313_801F2D60(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ + 0x12);
}
