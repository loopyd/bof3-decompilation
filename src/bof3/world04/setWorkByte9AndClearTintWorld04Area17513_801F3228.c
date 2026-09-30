#include "bof3/bof3.h"

/* @source 0x801F3228
 * @behavior Stores 0x10 into the byte at offset 0x09 and 1 into the byte at
 * offset 0x01 of the scratchpad work object published through the
 * non-volatile pointer cell at 0x1F800044, then zeroes that object's three
 * tint channel bytes at offsets 0x5F, 0x5E and 0x5D; takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9AndClearTintWorld04Area17513_801F3228(void) {
  u8** slots;
  u8* work;

  slots = SPAD_PTR_TABLE(u8);
  slots[0x11][9] = 0x10;
  slots[0x11][1] = 1;
  work = slots[0x11];
  work[0x5D] = work[0x5E] = work[0x5F] = 0;
}
