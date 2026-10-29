#include "bof3/bof3.h"

extern u8 D_8014933B;

/* @source 0x801F446C
 * @behavior Overlay setter: writes the constant 0x01 to the byte at 0x8014933B.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorld04Area16713_801F446C(void) {
  D_8014933B = 0x01;
}
