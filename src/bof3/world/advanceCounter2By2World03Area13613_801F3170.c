#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F3170
 * @behavior Overlay handler step: reads the shared halfword counter at
 * 0x8014932A, stores the constant 2 into the shared status byte at 0x80149333,
 * then writes the counter back advanced by 0x02. This is the advance half of
 * the pair whose retreat half is the adjacent
 * retreatCounter2By2World03Area13613_801F3148: same load-first idiom on the same
 * counter halfword with the step immediate negated, and both addresses are
 * adjacent .word entries at 0x801F437C and 0x801F4380 in this payload's handler
 * table. The 0x28-byte range is byte-identical to the already-exact
 * advanceCounter2By2World03Area11913_801F41B8 at its own 0x801F41B8 and to
 * advanceCounter2By2World04Area18813_801F48E0 at its own 0x801F48E0, which
 * carried exact matches when this seed was written, so this body is that
 * sibling's shape with this payload's own entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounter2By2World03Area13613_801F3170(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ + 0x02);
}
