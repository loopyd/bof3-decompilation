#include "bof3/bof3.h"

extern s16 D_8014932C;
extern u8 D_80149333;

/* @source 0x801F2D18
 * @behavior Overlay setter: stores the signed halfword -0x100 (0xFF00) into the
 * shared halfword D_8014932C at 0x8014932C and then the constant 0x02 into the
 * shared status byte D_80149333 at 0x80149333. Takes no arguments and returns
 * nothing; both immediates reach their stores through $v0 and no frame is built.
 * The 32-byte body is byte-identical to the span at 0x801F3E00 in
 * emi/world01/area041/13 (payload cmp, 32 of 32 bytes), which measured exact
 * when this seed was copied from it. No word of this payload calls or points at
 * the entry, so its argument and return roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateHalfwordFF00Two(void) {
  D_8014932C = -0x100;
  D_80149333 = 0x02;
}
