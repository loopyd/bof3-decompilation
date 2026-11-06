#include "bof3/bof3.h"

extern void* D_80146884; /* @source 0x80146884 @kind data */

/* @source 0x801F2E74
 * @behavior Clears bit 0 of the byte at offset 0x74 of the work record
 * published at 0x80146884. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkAnimBit0World01Area07413_801F2E74(void) {
  u8* record;

  record = D_80146884;
  record[0x74] &= 0xFE;
}
