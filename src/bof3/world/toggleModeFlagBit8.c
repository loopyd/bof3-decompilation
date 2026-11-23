#include "bof3/bof3.h"

extern u16 D_80146258;

/* @source 0x801F3B14
 * @behavior Flips bit 8 of the shared mode-flag halfword D_80146258 with one
 *           lhu/xori/sh read-modify-write, leaving every other bit of that
 *           halfword untouched; takes no arguments and returns nothing. Leaf
 *           with no frame and no calls: the store re-materialises the 0x8014
 *           page in $at because $v0 still holds the xored value, and both delay
 *           slots stay nop. The payload's data blob stores this address at
 *           0x801F4A64 inside a run of code and data pointers, so the overlay
 *           reaches the body as a table entry; which step that entry selects is
 *           not established, so the name stays anchored at 0x801F3B14.
 * @status exact
 * @match 100.00
 * @residual none
 */
void toggleModeFlagBit8(void) {
  D_80146258 ^= 8;
}
