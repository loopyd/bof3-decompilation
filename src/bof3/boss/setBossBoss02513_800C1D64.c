#include "bof3/bof3.h"

extern u8 D_80146384;

/* @source 0x800C1D64
 * @behavior Overlay setter: writes the constant 0x04 to the byte at 0x80146384.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setBossBoss02513_800C1D64(void) {
  D_80146384 = 0x04;
}
