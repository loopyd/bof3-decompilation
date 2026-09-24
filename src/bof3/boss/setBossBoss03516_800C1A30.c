#include "bof3/bof3.h"

extern u8 D_80146384;

/* @source 0x800C1A30
 * @behavior Overlay setter: writes the constant 0x03 to the byte at 0x80146384.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setBossBoss03516_800C1A30(void) {
  D_80146384 = 0x03;
}
