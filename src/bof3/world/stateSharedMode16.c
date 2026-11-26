#include "bof3/bof3.h"

extern u8 D_8014933B;

/* @source 0x801F4480
 * @behavior Overlay setter: writes the constant 0x10 to the byte at 0x8014933B.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSharedMode16(void) {
  D_8014933B = 0x10;
}
