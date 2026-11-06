#include "bof3/bof3.h"

extern void* D_80146884; /* @source 0x80146884 @kind data */

/* @source 0x801F2D54
 * @behavior Clears bit 0 of the byte at offset 0x74 of the work record
 * published at 0x80146884, then clears byte 0 of the scratchpad work record
 * published at the pointer cell 0x1F800044. Takes no arguments and returns
 * nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkAnimBit0ClearScratchWorkByte0World00Area03513_801F2D54(void) {
  u8* record;

  record = D_80146884;
  record[0x74] &= 0xFE;
  SPAD_PTR_TABLE(u8)[0x11][0] = 0;
}
