#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F4AF4
 * @behavior Clears flag bit 0x20 of byte 0 of the scratch work record reached
 * through the pointer cell at 0x1F800044, writes 0 into the record's 0x5C
 * marker byte, and then reloads the cell to write 0 into each of the signed
 * level bytes 0x5F, 0x5E and 0x5D in that order. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void disarmScratchWorkLevelTripleWorld04Area17213_801F4AF4(void) {
  u8* record;

  D_1F800044[0] &= 0xdf;
  D_1F800044[0x5c] = 0;
  record = D_1F800044;
  record[0x5f] = 0;
  record[0x5e] = 0;
  record[0x5d] = 0;
}
