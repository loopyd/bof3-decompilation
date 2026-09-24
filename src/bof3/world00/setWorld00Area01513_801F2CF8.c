#include "bof3/bof3.h"

extern u8 D_8014832E;

/* @source 0x801F2CF8
 * @behavior Overlay setter: writes the constant 0x1F to the byte at 0x8014832E.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorld00Area01513_801F2CF8(void) {
  D_8014832E = 0x1F;
}
