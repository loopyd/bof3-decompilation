#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F3148
 * @behavior Overlay counter step: reads the shared halfword counter at
 * 0x8014932A, stores the constant 2 into the shared status byte at 0x80149333,
 * then writes the counter back retreated by 0x02. It mirrors the adjacent step
 * at 0x801F3170 in this same payload, which advances that counter by the same
 * amount: both bodies keep this load-first order and differ only in the sign of
 * the step immediate. The 0x28-byte range is byte-identical to the already-exact
 * retreatCounter2By2World03Area11913_801F41E0 in emi/world03/area119/13 at its
 * own 0x801F41E0, so this body is that sibling's shape with this payload's
 * literals.
 * @status exact
 * @match 100.00
 * @residual none
 */
void retreatCounter2By2World03Area13613_801F3148(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ - 0x02);
}
