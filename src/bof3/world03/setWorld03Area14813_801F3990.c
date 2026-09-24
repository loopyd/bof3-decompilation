#include "bof3/bof3.h"

extern u8 D_8014933B;

/* @source 0x801F3990
 * @behavior Overlay setter: writes the constant 0x01 to the byte at 0x8014933B.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorld03Area14813_801F3990(void) {
  D_8014933B = 0x01;
}
