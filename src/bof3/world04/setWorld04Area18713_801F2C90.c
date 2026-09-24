#include "bof3/bof3.h"

extern u8 D_801448EB;

/* @source 0x801F2C90
 * @behavior Overlay setter: writes the constant 0x1D to the byte at 0x801448EB.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorld04Area18713_801F2C90(void) {
  D_801448EB = 0x1D;
}
