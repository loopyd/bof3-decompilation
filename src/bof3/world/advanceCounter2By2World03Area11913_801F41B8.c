#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F41B8
 * @behavior Overlay counter step: reads the shared halfword counter at
 * 0x8014932A, stores the constant 2 into the shared status byte at 0x80149333,
 * then writes the counter back advanced by 0x02. This is the advance half of
 * the pair whose retreat half is the adjacent
 * retreatCounter2By2World03Area11913_801F41E0: same load-first idiom and same
 * counter halfword with the immediate negated. Both addresses appear as
 * adjacent .word entries at 0x801F52F0 in the payload's pointer table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounter2By2World03Area11913_801F41B8(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ + 0x02);
}
